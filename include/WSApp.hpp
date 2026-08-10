#ifndef WSAPP_H
# define WSAPP_H

# include "Config.hpp"
# include "Poller.hpp"
# include "Server.hpp"

# include <csignal>
# include <vector>
# include <map>

class WSApp
{
private:
	Config const *					_conf;
	Poller							_pol;
	std::vector<Server *>			_servs;
	static volatile sig_atomic_t	g_shutdownReq;

	struct epoll_res {
		int								n;
		struct epoll_event 	const *		epEv;
	}	_epRes;

	void		addServ(ServerDirective const &servDir);
	void		ep_regisListeners();
	int			ep_wait();

	int		hndl_Lis(eventCtx *ctx);
	int		hndl_Cli(eventCtx *ctx, uint32_t events);

	WSApp(WSApp const & other);
	WSApp &	operator=(WSApp const & other);

public:
	WSApp();
	~WSApp();

	void		ConfigInit(Config const &conf);
	int			run();
};

#endif
