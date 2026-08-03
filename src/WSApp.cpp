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

// Private --------------------------------------------------------------------

// ----------------------------------------------------------------------------

void	WSApp::addServ(std::string const &host, int port)
{
	Server *	new_serv = new Server(host, port);
	_servs.push_back(new_serv);
}

void	WSApp::regisListeners()
{
	for (std::vector<Server *>::iterator it = _servs.begin();
		it != _servs.end(); ++it)
	{
		Listener &	lis = (*it)->listener();
		_p.add(lis.fd(), EPOLLIN, &lis.ectx());
	}
}

epoll_res	WSApp::wait4events(int timeout_ms)
{
	int rdy_n = _p.wait(timeout_ms);
	struct epoll_event const *	rdy_events = _p.rdy_events();

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

	return _p.add(client_fd, EPOLLIN, &cli->ectx());
	//rm client from server if add fails...
}

int		WSApp::hndl_Cli(eventCtx *ctx, uint32_t events)
{
	Client &	cli = *(static_cast<Client *>(ctx->owner));

	if (events & EPOLLIN)
	{
		if (cli.isClosing())
			return 0;
		int ret = cli.recv1();
		if (ret <= 0)
			cli.toClose();
	}

	if (events & EPOLLOUT)
	{
		if (cli.isClosing())
			return 0;
		int ret = cli.send1();
		(void)ret;
		if (!cli.isSending())
			_p.mod(cli.fd(), EPOLLIN, &cli.ectx());
	}

	return 0;
}
