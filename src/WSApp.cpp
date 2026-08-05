#include "WSApp.hpp"

volatile sig_atomic_t	WSApp::g_shutdownReq = 0;

// OCF ------------------------------------------------------------------------

WSApp::WSApp() {}

WSApp::~WSApp()
{
	for (std::vector<Server *>::iterator it = _servs.begin();
		it != _servs.end(); ++it)
	delete *it;
}

// ----------------------------------------------------------------------------

int		WSApp::run()
{
	regisListeners();
	while (1)
	{
		epoll_res	rdy = wait(10000);
		if (rdy.n == -1)
			return 1;

		for (int i = 0; i < rdy.n; ++i)
		{
			eventCtx *	ctx = static_cast<eventCtx *>(rdy.ep_ev[i].data.ptr);
			switch (ctx->type)
			{
				case SCK_LISTENER:
					hndl_Lis(ctx);
					break;
				case SCK_CLIENT:
					hndl_Cli(ctx, rdy.ep_ev[i].events);
					break;
				default:
					break;
			}
		}

		std::vector<Server *>::iterator it = _servs.begin();
		while (it != _servs.end())
		{
			Server &	s = **it;
			client_map_t::iterator i = s.clients().begin();
			while (i != s.clients().end())
			{
				Client & cli = i->second;
				if (cli.isStat(CLOSING))
				{
					int cls_fd = cli.fd();
					++i;
					_pol.del(cls_fd);
					s.rmClient(cls_fd);
					continue;
				}
				std::string const & inbox = cli.readInbox();
				if (inbox.size() >= 10 && !cli.isStat(SENDING))
				{
					std::string tmp_resp = "Responding to inbox size 10\n";
					cli.tmp_req_for_resp(10, tmp_resp);
					_pol.mod(cli.fd(), EPOLLOUT | EPOLLIN, &cli.ectx());
				}
				++i;
			}
			++it;
		}
	}
}

void	WSApp::addServ(std::string const &host, int port)
{
	Server *	new_serv = new Server(host, port);
	_servs.push_back(new_serv);
}

// Private --------------------------------------------------------------------

// to check return from epoll add
void	WSApp::regisListeners()
{
	for (std::vector<Server *>::iterator it = _servs.begin();
		it != _servs.end(); ++it)
	{
		Listener &	lis = (*it)->listener();
		_pol.add(lis.fd(), EPOLLIN, &lis.ectx());
	}
}

epoll_res	WSApp::wait(int timeout_ms)
{
	int rdy_n = _pol.wait(timeout_ms);
	struct epoll_event const *	rdy_events = _pol.rdy_events();

	epoll_res res = {rdy_n, rdy_events};
	return res;
}

int		WSApp::hndl_Lis(eventCtx *ctx)
{
	Listener &	lis = *(static_cast<Listener *>(ctx->owner));
	Server &	ser = lis.server();

	int client_fd = lis.welcome();
	if (client_fd == -1)
		return -1;

	Client * 	cli = ser.addClient(client_fd);
	if (!cli)
		return -1;

	return _pol.add(client_fd, EPOLLIN, &cli->ectx());
	//rm client from server if add fails...
}

int		WSApp::hndl_Cli(eventCtx *ctx, uint32_t events)
{
	Client &	cli = *(static_cast<Client *>(ctx->owner));

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
