#include "Server.hpp"

#include <utility>
#include <iostream>

// OCF ------------------------------------------------------------------------

Server::Server(std::string const &host, int port)
: _host(host), _port(port), _listener(*this, host, port) {}

Server::~Server() {}

// Private --------------------------------------------------------------------

// ----------------------------------------------------------------------------

Listener &		Server::listener() {
	return _listener;
}

bool			Server::addClient(int fd)
{
	std::pair<client_map_t::iterator, bool>	kv;

	kv = _clients.insert(std::make_pair(fd, Client()));
	if (!kv.second)
	{
		std::cerr 	<< "[WARNING] Attempt to add Client with duplicate fd: "
					<< fd << std::endl;
		close(fd);
		return false;
	}
	kv.first->second.initClient(fd, this);
	return true;
}

Client	*		Server::client(int fd)
{
	client_map_t::iterator it; 

	it = _clients.find(fd);
	if (it == _clients.end())
		return NULL;
	return &it->second;
}
