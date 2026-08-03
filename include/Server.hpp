#ifndef SERVER_H
# define SERVER_H

#include "Listener.hpp"
#include "Client.hpp"

#include <string>
#include <map>

typedef std::map<int, Client>	client_map_t;

class Server
{
private:
	std::string				_host;
	int						_port;
	std::string				_name;

	Listener				_listener;
	client_map_t			_clients;

	// No default construction
	// No copying; so for container of multiple servers, use vector<Server *>
	Server();
	Server(Server const & other);
	Server &	operator=(Server const & other);

public:
	Server(std::string const &host, int port);
	~Server();

	std::string	const &		name()		{ return _name; }
	Listener &				listener()	{ return _listener; }

	client_map_t &	clients() { return _clients; }
	Client	*		client(int fd);

	Client *		addClient(int fd);
	bool			rmClient(int fd);
};

#endif
