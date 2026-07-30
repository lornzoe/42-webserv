#ifndef SERVER_H
# define SERVER_H

#include "Listener.hpp"
#include "Client.hpp"

#include <string>
#include <map>

class Server
{
private:
	std::string					_host;
	int							_port;

	Listener					_listener;
	std::map<int, Client>		_clients;

	Server();
	Server(Server const & other);
	Server &	operator=(Server const & other);

public:
	Server(std::string const &host, int port);
	~Server();

	Listener &		listener();
	void			addClient(int fd);
	Client	&		getClient(int fd);
};

#endif
