#include "Server.hpp"
#include "w_utils.hpp"
#include "w_logger.hpp"
#include <iostream>

#include <utility>

// OCF ------------------------------------------------------------------------

Server::Server(std::string const &host, int port)
: _host(host), _port(port), _name("(" + host + ":" + wutils::ft_itoa(port) + ")"),
	_listener(*this, host, port)
{ LOG_INFO("Server " << _name << " created"); }

Server::~Server() {}

// Private --------------------------------------------------------------------

// ----------------------------------------------------------------------------

bool			Server::addClient(int fd)
{
	std::pair<client_map_t::iterator, bool>	kv;

	kv = _clients.insert(std::make_pair(fd, Client()));
	if (!kv.second)
	{
		LOG_WARN("Cannot add duplicate Client fd " << fd << " to Server " << _name);
		close(fd);
		return false;
	}
	kv.first->second.initClient(*this, fd);
	LOG_INFO("Added Client fd " << fd << " to Server " << _name);
	return true;
}

//remove client from server storage
//cleanup of client is isolated to the client object destructor
bool		Server::rmClient(int fd)
{
	int number_erased = _clients.erase(fd);
	if (number_erased != 1)
	{
		LOG_WARN("Cannot rm Client fd " << fd << " froms Server " << _name);
		return false;
	}
	LOG_INFO("Rm Client fd " << fd << " from Server " << _name);
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
