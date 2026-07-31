#ifndef LISTENER_H
# define LISTENER_H

#include "w_eventCtx.hpp"

#include <string>

class Server;

class Listener
{
private:
	Server &		_server;
	int				_fd;
	eventCtx		_eCtx;

	static int		noBlock(int fd);
	static int		reuseAddr(int fd);

	Listener();
	
public:
	Listener(Server &server, std::string const &host, int port);
	~Listener();

	Server &		server()	{ return _server; }
	int				fd() 		{ return _fd; }
	eventCtx &		ectx() 		{ return _eCtx; }

	int				welcome() const;
};

#endif
