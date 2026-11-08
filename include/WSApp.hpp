#ifndef WSAPP_H
#define WSAPP_H

#include "Config.hpp"
#include "Poller.hpp"
#include "Server.hpp"

#include <csignal>
#include <vector>
#include <map>
#include <cerrno>

class WSApp
{
private:
	Config const *_conf;
	Poller _pol;
	std::vector<Server *> _servs;

	struct epoll_res
	{
		int n;
		struct epoll_event const *epEv;
	} _epRes;

	void addServ(ServerDirective const &servDir);
	void ep_regisListeners();
	int ep_wait();
	void sesh_cleanup(std::time_t &last_sesh_cleanup);

	int hndl_Lis(eventCtx *ctx);
	int hndl_Cli(eventCtx *ctx, uint32_t events);
	int handle_cgi_in(eventCtx *ctx);
	int handle_cgi_out(eventCtx *ctx);

	Client *findCgiClient(CgiProcess &cgi_process);

	WSApp(WSApp const &other);
	WSApp &operator=(WSApp const &other);

	static const int CLIENT_TIMEOUT_SEC;
	static const int SESH_TIMEOUT_SEC = 900;
	static const int CGI_TIMEOUT_SECOND;

	void cgi_cleanup(CgiProcess &cgi);
	void cgi_abort(Client &cli, int code);
	int cgi_finish(Client &cli);

public:
	WSApp();
	~WSApp();

	void ConfigInit(Config const &conf);
	int run();
};

#endif
