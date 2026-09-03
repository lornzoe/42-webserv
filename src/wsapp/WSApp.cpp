#include "WSApp.hpp"
#include "ServerDirective.hpp"
#include "ListenDirective.hpp"
#include "FileDescriptor.hpp"
#include "w_logger.hpp"

#include <string>
#include <sstream>
#include <sys/stat.h>

volatile sig_atomic_t WSApp::g_shutdownReq = 0;

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
	ep_regisListeners();
	while (1)
	{
		ep_wait();
		if (_epRes.n == -1)
			return 1;

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
				if (cli.isStat(CLOSING))
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
					ParseResult result = HttpRequest::parse_http_request(inbox);
					if (result.status == INCOMPLETE)
					{
						// TODO: Handle timeout
					}
					if (result.status == INVALID)
					{
						// TODO: close connection
					}
					if (result.status == COMPLETE)
					{
						cli.process_request(result);
						_pol.mod(cli.fd(), EPOLLOUT | EPOLLIN, &cli.ectx());
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

	int client_fd = lis.welcome();
	if (client_fd == -1)
		return -1;

	Client *cli = ser.addClient(client_fd);
	if (!cli)
		return -1;

	return _pol.add(client_fd, EPOLLIN, &cli->ectx());
	// rm client from server if add fails...
}

int WSApp::hndl_Cli(eventCtx *ctx, uint32_t events)
{
	Client &cli = *(static_cast<Client *>(ctx->owner));

	if (events & EPOLLIN)
	{
		if (cli.isStat(CLOSING))
			return 0;
		int ret = cli.recv1();
		if (ret <= 0)
			cli.addStat(CLOSING);
	}

	if (events & EPOLLOUT)
	{
		if (cli.isStat(CLOSING))
			return 0;
		int ret = cli.send1();
		(void)ret;
		if (!cli.isStat(SENDING))
			_pol.mod(cli.fd(), EPOLLIN, &cli.ectx());
	}

	return 0;
}
