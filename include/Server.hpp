#ifndef SERVER_H
# define SERVER_H

#include "Listener.hpp"
#include "Client.hpp"

#include <string>
#include <map>

class Server
{
private:
	std::string				_host;
	int						_port;
	std::string				_name;

	Listener				_listener;

	typedef std::map<int, Client>	client_map_t;
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

	Client	*		client(int fd);

	bool			addClient(int fd);
	bool			rmClient(int fd);
};

#endif
