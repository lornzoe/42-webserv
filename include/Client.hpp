#ifndef CLIENT_H
# define CLIENT_H

#include "w_eventCtx.hpp"

# include <unistd.h>
# include <string>

class Server;

class Client
{
private:
	Server *		_server;
	int				_fd;
	eventCtx		_eCtx;

	std::string		_inbox;
	std::string		_outbox;

public:
	Client();
	~Client();

	//faux copy constructor & assignment equivalent to default
	Client(Client const & other) : _fd(-1) { (void)other; }
	Client &	operator=(Client const &other) { if (this != &other) _fd = -1; return *this; }

	Server &		server()	{ return *_server; }
	int				fd() 		{ return _fd; }
	eventCtx &		ectx()		{ return _eCtx; }

	void			initClient(Server &server, int fd);
};

#endif
