#ifndef LISTENER_H
# define LISTENER_H

#include "EventCtx.hpp"

#include <string>

class Server;

class Listener
{
private:
	int					_fd;
	Server const &		_server;
	EventCtx			_ctx;

	static int		noBlock(int fd);
	static int		reuseAddr(int fd);

	Listener();
	
public:
	Listener(Server const &server, std::string const &host, int port);
	~Listener();

	int				fd();
	EventCtx *		ctx();
	int				regisClient();
};

#endif
