#include "WSApp.hpp"
#include "ServerDirective.hpp"
#include "ListenDirective.hpp"
#include "FileDescriptor.hpp"
#include "HttpRequest.hpp"
#include "ReqProc.hpp"
#include "HttpResponse.hpp"
#include "w_logger.hpp"

#include <string>
#include <sstream>
#include <sys/stat.h>
#include <csignal>

namespace
{

	volatile sig_atomic_t g_shutdownRequest = false;

	void sigint_handler(int sig)
	{
		(void)sig;
		g_shutdownRequest = true;
	}

}

const int WSApp::TIMEOUT_SECOND = 5;
const int WSApp::CGI_TIMEOUT_SECOND = 10;

// OCF ------------------------------------------------------------------------

WSApp::WSApp() {}

WSApp::~WSApp()
{
	for (std::vector<Server *>::iterator it = _servs.begin();
		 it != _servs.end(); ++it)
		delete *it;
}

// ----------------------------------------------------------------------------

void WSApp::ConfigInit(Config const &conf)
{
	_conf = &conf;
	std::vector<const ServerDirective *> servers = conf.getServerDirectives();
	for (std::size_t i = 0; i < servers.size(); i++)
		addServ(*servers[i]);
}

int WSApp::run()
{
	signal(SIGINT, sigint_handler);
	signal(SIGPIPE, SIG_IGN);
	ep_regisListeners();
	while (!g_shutdownRequest)
	{
		ep_wait();
		if (_epRes.n == -1)
		{
			if (!g_shutdownRequest)
				return 1;
			break;
		}

		for (int i = 0; i < _epRes.n; ++i)
		{
			eventCtx *ctx = static_cast<eventCtx *>(_epRes.epEv[i].data.ptr);
			switch (ctx->type)
			{
			case SCK_LISTENER:
				hndl_Lis(ctx);
				break;
			case SCK_CLIENT:
				hndl_Cli(ctx, _epRes.epEv[i].events);
				break;
			case CGI_IN:
				handle_cgi_in(ctx);
				break;
			case CGI_OUT:
				handle_cgi_out(ctx);
				break;
			default:
				break;
			}
		}

		std::vector<Server *>::iterator it = _servs.begin();
		while (it != _servs.end())
		{
			Server &s = **it;
			Server::client_map_t::iterator i = s.clients().begin();
			while (i != s.clients().end())
			{
				Client &cli = i->second;
				if (cli.isStat(CLOSING) && !cli.isStat(SENDING) && !cli.isStat(WAIT_CGI))
				{
					int cls_fd = cli.fd();
					++i;
					_pol.del(cls_fd);
					s.rmClient(cls_fd);
					continue;
				}

				if (cli.isStat(WAIT_CGI))
				{
					if (time(NULL) - cli.get_cgi_process().startTime() >= CGI_TIMEOUT_SECOND)
					{
						LOG_DEBUG("CGI TIMEOUT for client " << cli.fd());
						cgi_abort(cli, 504);
					}
					++i;
					continue;
				}

				if (!cli.isStat(CLOSING) &&
					time(NULL) - cli.get_last_activity_time() >= TIMEOUT_SECOND)
				{
					if (cli.isStat(SENDING))
					{
						// Client stopped reading our response: just drop it.
						cli.rmStat(SENDING);
						cli.addStat(CLOSING);
					}
					else if (cli.readInbox().empty())
					{
						// Idle keep-alive connection, nothing pending: close quietly.
						cli.addStat(CLOSING);
					}
					// partial request in the inbox, handled by INCOMPLETE branch (408)
					++i;
					continue;
				}

				std::string const &inbox = cli.readInbox();
				if (inbox.size() > 0 && !cli.isStat(SENDING))
				{
					ParseResult res = HttpRequest::parse_http_request(inbox);
					LOG_DEBUG("Parsed HTTP request with status: " << res.status);
					if (res.status == INCOMPLETE)
					{
						LOG_DEBUG("HTTP request is incomplete, waiting for more data.");
						time_t current_time = time(NULL);
						if (current_time - cli.get_last_activity_time() >= TIMEOUT_SECOND)
						{
							LOG_DEBUG("TIMEOUT FOR CLIENT " << cli.fd());
							cli.send_response(HttpResponse::buildError(res.errorCode, res.request.path, &cli.servDir()));
							_pol.mod(cli.fd(), EPOLLOUT, &cli.ectx());
							cli.addStat(CLOSING);
						}
					}
					else if (res.status == INVALID)
					{
						LOG_DEBUG("HTTP request is invalid, sending error response with code: " << res.errorCode);
						cli.send_response(HttpResponse::buildError(res.errorCode, res.request.path, &cli.servDir()));
						_pol.mod(cli.fd(), EPOLLOUT, &cli.ectx());
						cli.addStat(CLOSING);
					}
					else
					{
						LOG_DEBUG("HTTP request is complete, processing request.");
						try
						{
							if (cli.process_request(res) == ReqProc::RESP_RDY)
							{
								LOG_DEBUG("HTTP response prepared, switching to EPOLLOUT for sending.");
								_pol.mod(cli.fd(), EPOLLOUT, &cli.ectx());
								LOG_DEBUG("Finished handling client request.");
							}
							else
							{
								cli.addStat(WAIT_CGI);
								_pol.add(cli.get_cgi_process().getCgiReadFd(), EPOLLIN, &cli.get_cgi_process().get_in_ectx());
								if (res.request.method == "POST" && !cli.get_cgi_process().getRequestBody().empty())
								{
									_pol.add(cli.get_cgi_process().getCgiWriteFd(), EPOLLOUT, &cli.get_cgi_process().get_out_ectx());
								}
								else
								{
									// No request body to send.
									// Close the parent's write end so the CGI child receives EOF.
									cli.get_cgi_process().closeCgiWriteFd();
								}
								_pol.del(cli.fd());
							}
						}
						catch (const CgiProcess::ChildProcessException &)
						{
							throw; // child only: unwind all the way to main, never touch parent state
						}
						catch (const std::exception &e)
						{
							LOG_DEBUG("CGI start failed: " << e.what());
							cli.get_cgi_process().killAndReap(); // safe: does nothing if no child
							cli.rmStat(WAIT_CGI);				 // in case it was set
							cli.send_response(HttpResponse::buildError(500, res.request.path, &cli.servDir()));
							_pol.mod(cli.fd(), EPOLLOUT, &cli.ectx());
							cli.addStat(CLOSING);
						}
					}
				}
				++i;
			}
			++it;
		}
	}
	return 0;
}

// Private --------------------------------------------------------------------

void WSApp::addServ(ServerDirective const &servDir)
{
	std::vector<const ListenDirective *> listenDirs = servDir.getListens();
	for (std::size_t i = 0; i < listenDirs.size(); ++i)
	{
		_servs.push_back(new Server(servDir,
									listenDirs[i]->getHost(),
									listenDirs[i]->getPort()));
	}
}

// to check return from epoll add
void WSApp::ep_regisListeners()
{
	for (std::vector<Server *>::iterator it = _servs.begin();
		 it != _servs.end(); ++it)
	{
		Listener &lis = (*it)->listener();
		_pol.add(lis.fd(), EPOLLIN, &lis.ectx());
	}
}

int WSApp::ep_wait()
{
	_epRes.n = _pol.wait();
	_epRes.epEv = _pol.rdy_events();

	return _epRes.n;
}

int WSApp::hndl_Lis(eventCtx *ctx)
{
	Listener &lis = *(static_cast<Listener *>(ctx->owner));
	Server &ser = lis.server();

	remote_info rinfo;
	int client_fd = lis.welcome(rinfo);
	if (client_fd == -1)
		return -1;

	Client *cli = ser.addClient(client_fd, rinfo);
	if (!cli)
		return -1;

	return _pol.add(client_fd, EPOLLIN, &cli->ectx());
	// rm client from server if add fails...
}

int WSApp::hndl_Cli(eventCtx *ctx, uint32_t events)
{
	Client &cli = *(static_cast<Client *>(ctx->owner));

	if (events & EPOLLIN && !cli.isStat(CLOSING))
	{
		int ret = cli.recv1();
		if (ret <= 0)
			cli.addStat(CLOSING);
	}

	if (events & EPOLLOUT && cli.isStat(SENDING))
	{
		int ret = cli.send1();
		if (ret < 0)
		{
			cli.rmStat(SENDING);
			cli.addStat(CLOSING);
		}
		if (!cli.isStat(SENDING) && !cli.isStat(CLOSING))
			_pol.mod(cli.fd(), EPOLLIN, &cli.ectx());
	}

	return 0;
}

int WSApp::handle_cgi_in(eventCtx *ctx)
{
	CgiProcess &cgi_process = *(static_cast<CgiProcess *>(ctx->owner));

	int cgi_read_fd = cgi_process.getCgiReadFd();
	if (cgi_read_fd < 0)
		return 0;

	// Find the associated Client.
	Client *client = findCgiClient(cgi_process);
	if (!client)
	{
		// The client disconnected or no association exists.
		cgi_cleanup(cgi_process);
		return -1;
	}

	char buffer[4096];
	ssize_t bytes_read = read(
		cgi_read_fd,
		buffer,
		sizeof(buffer));

	if (bytes_read > 0)
	{
		cgi_process.appendOutbox(
			std::string(buffer, static_cast<size_t>(bytes_read)));
		return 0;
	}

	if (bytes_read == 0)
	{
		_pol.del(cgi_read_fd);
		cgi_process.closeCgiReadFd();

		return cgi_finish(*client);
	}

	// No data is currently available, wait for the next readiness notification
	if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
		return 0;

	// A genuine read error occurred.
	cgi_abort(*client, 502);

	return -1;
}

int WSApp::handle_cgi_out(eventCtx *ctx)
{
	CgiProcess &cgi_process = *(static_cast<CgiProcess *>(ctx->owner));

	int fd = cgi_process.getCgiWriteFd();
	if (fd < 0)
		return 0;

	std::string const &body = cgi_process.getRequestBody();
	size_t &offset = cgi_process.getBodyWriteOffset();

	// No body, or the entire body has already been sent.
	if (offset >= body.size())
	{
		_pol.del(fd);
		cgi_process.closeCgiWriteFd();
		return 0;
	}

	ssize_t bytes_written = write(
		fd,
		body.data() + offset,
		body.size() - offset);

	if (bytes_written > 0)
	{
		offset += static_cast<size_t>(bytes_written);

		// The CGI child has received the entire request body.
		if (offset == body.size())
		{
			_pol.del(fd);
			cgi_process.closeCgiWriteFd();
		}

		return 0;
	}

	if (bytes_written == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
			return 0;
	}

	// write() returned 0 unexpectedly, or a non-retryable error occurred.
	_pol.del(fd);
	cgi_process.closeCgiWriteFd();

	return -1;
}

Client *WSApp::findCgiClient(CgiProcess &cgi_process)
{
	for (std::vector<Server *>::iterator it = _servs.begin();
		 it != _servs.end(); ++it)
	{
		Server &server = **it;

		Server::client_map_t::iterator client_it =
			server.clients().begin();

		while (client_it != server.clients().end())
		{
			Client &client = client_it->second;

			if (&client.get_cgi_process() == &cgi_process)
				return &client;

			++client_it;
		}
	}

	return NULL;
}

// Close both CGI fds (if open) and make sure the child is dead and reaped.
void WSApp::cgi_cleanup(CgiProcess &cgi)
{
	int rfd = cgi.getCgiReadFd();
	if (rfd >= 0)
	{
		_pol.del(rfd);
		cgi.closeCgiReadFd();
	}

	int wfd = cgi.getCgiWriteFd();
	if (wfd >= 0)
	{
		_pol.del(wfd);
		cgi.closeCgiWriteFd();
	}

	cgi.killAndReap();
}

// Something went wrong: clean up, tell the client, close the connection.
void WSApp::cgi_abort(Client &cli, int code)
{
	cgi_cleanup(cli.get_cgi_process());
	cli.rmStat(WAIT_CGI);
	cli.send_response(HttpResponse::buildError(code, "", &cli.servDir()));
	_pol.add(cli.fd(), EPOLLOUT, &cli.ectx());
	cli.addStat(CLOSING);
}

// Called ONLY when the CGI's stdout reached EOF. Output is complete.
int WSApp::cgi_finish(Client &cli)
{
	CgiProcess &cgi = cli.get_cgi_process();

	// Stop sending the request body if we still were.
	int wfd = cgi.getCgiWriteFd();
	if (wfd >= 0)
	{
		_pol.del(wfd);
		cgi.closeCgiWriteFd();
	}

	// NOW check the exit status.
	int status = 0;
	if (!cgi.tryReap(status))
		cgi.killAndReap(); // closed stdout but still running: stop it
	else if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
	{
		cgi_abort(cli, 502);
		return -1;
	}

	std::string resp;
	if (!HttpResponse::buildCgiResponse(cgi.getOutbox(), resp))
	{
		cgi_abort(cli, 502);
		return -1;
	}
	if (cli.isStat(CLOSING)) // request had "Connection: close"
		resp.insert(resp.find(HEADER_END), CRLF + "Connection: close");

	cli.rmStat(WAIT_CGI);
	cli.send_response(resp);
	_pol.add(cli.fd(), EPOLLOUT, &cli.ectx());
	return 0;
}
