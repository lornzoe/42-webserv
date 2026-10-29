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
				if (cli.isStat(CLOSING) && !cli.isStat(SENDING))
				{
					int cls_fd = cli.fd();
					++i;
					_pol.del(cls_fd);
					s.rmClient(cls_fd);
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
						if (cli.process_request(res) == ReqProc::RESP_RDY)
						{
							LOG_DEBUG("HTTP response prepared, switching to EPOLLOUT for sending.");
							_pol.mod(cli.fd(), EPOLLOUT, &cli.ectx());
							LOG_DEBUG("Finished handling client request.");
						}
						else
						{
							_pol.add(cli.get_cgi_process().getCgiReadFd(), EPOLLIN, &cli.get_cgi_process().get_in_ectx());
							if (res.request.method == "POST" &&
								res.request.content_length > 0)
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
				}
				// if is WAIT_CGI
				// if complete
				// package response and send on cli outbox
				// update EPOLL and client status
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

// TODO: The CGI child process fails or exits unsuccessfully.
// Reading CGI output (handle_cgi_in) fails.
// Writing the request body to CGI stdin (handle_cgi_out) fails.
int WSApp::handle_cgi_in(eventCtx *ctx)
{
	CgiProcess &cgi_process = *(static_cast<CgiProcess *>(ctx->owner));

	int cgi_read_fd = cgi_process.getCgiReadFd();
	if (cgi_read_fd < 0)
		return 0;

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

		// Find the associated Client.
		Client *client = findCgiClient(cgi_process);
		if (!client)
		{
			// The client disconnected or no association exists.
			return -1;
		}

		// Parse the complete CGI response.
		std::string const &cgi_output = cgi_process.getOutbox();

		size_t separator = cgi_output.find("\r\n\r\n");
		size_t separator_length = 4;

		if (separator == std::string::npos)
		{
			// Invalid CGI response: no header/body separator.
			// Handle this as a server-side error.
			// TODO:
			// client->send_response(
			// 	HttpResponse::buildError(
			// 		500, client->getRequestPath(), &client->servDir()));

			// _pol.mod(client->fd(), EPOLLOUT, &client->ectx());
			// client->addStat(CLOSING);
			return -1;
		}

		// TODO: Extract the status code
		int code = 200;
		std::string header = cgi_output.substr(0, separator);
		std::string body = cgi_output.substr(separator + separator_length);

		// TODO: Build the HTTP response.
		std::string response = HttpResponse::buildCgiResponse(code, header, body);

		// Register the client fd for EPOLLOUT.
		client->send_response(response);
		_pol.add(client->fd(), EPOLLOUT, &client->ectx());

		return 0;
	}

	// No data is currently available, wait for the next readiness notification
	if (errno == EAGAIN || errno == EWOULDBLOCK)
		return 0;

	if (errno == EINTR)
		return 0;

	// A genuine read error occurred.
	_pol.del(cgi_read_fd);
	cgi_process.closeCgiReadFd();

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
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return 0;

		if (errno == EINTR)
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