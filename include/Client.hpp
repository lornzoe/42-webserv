#ifndef CLIENT_H
# define CLIENT_H

#include "EventCtx.hpp"

# include <unistd.h>
# include <string>

class Server;

class Client
{
private:
	int					_fd;
	Server const *		_server;
	EventCtx			_ctx;

	// std::string		_inbox;
	// std::string		_outbox;

public:
	Client();
	~Client();
	Client(Client const & other);
	Client &	operator=(Client const & other);

	std::string		_inbox;
	std::string		_outbox;

	void			initClient(int fd, Server const * server);
	int				fd();
	EventCtx *		ctx();
};

#endif
