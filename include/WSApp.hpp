#ifndef WSAPP_H
# define WSAPP_H

# include "Poller.hpp"
# include "Server.hpp"

# include <csignal>
# include <vector>
# include <map>

struct epoll_res {
	int								n;
	struct epoll_event 	const *		ep_ev;
};

class WSApp
{
private:
	Poller							_pol;
	std::vector<Server *>			_servs;
	static volatile sig_atomic_t	g_shutdownReq;

	void		regisListeners();
	epoll_res	wait(int timeout_ms);

	int		hndl_Lis(eventCtx *ctx);
	int		hndl_Cli(eventCtx *ctx, uint32_t events);

	WSApp(WSApp const & other);
	WSApp &	operator=(WSApp const & other);

public:
	WSApp();
	~WSApp();

	//to eventually pass the entire config object, and construct all
	void		addServ(std::string const &host, int port);
	int			run();
};

#endif
