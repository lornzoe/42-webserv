#ifndef WSAPP_H
# define WSAPP_H

# include "Poller.hpp"
# include "Server.hpp"

# include <csignal>
# include <vector>

struct epoll_res {
	int								n;
	struct epoll_event 	const *		ep_res;
};

class WSApp
{
private:
	Poller					_p;
	std::vector<Server *>	_servs;
	
	static volatile sig_atomic_t	g_shutdownReq;

	WSApp(WSApp const & other);
	WSApp &	operator=(WSApp const & other);

public:
	WSApp();
	~WSApp();

	void		addServ(std::string const &host, int port);
	void		regisListeners();
	epoll_res	wait4events(int timeout_ms);

	int		hndl_Lis(eventCtx *ctx);
	int		hndl_Cli(eventCtx *ctx, uint32_t events);

	Poller &							poller() { return _p; }
	std::vector<Server *>::iterator		servBgn() { return _servs.begin(); }
	std::vector<Server *>::iterator		servEnd() { return _servs.end(); }
};

#endif
