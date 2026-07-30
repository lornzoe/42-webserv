#include "Server.hpp"

// OCF ------------------------------------------------------------------------

Server::Server(std::string const &host, int port)
: _host(host), _port(port), _listener(*this, host, port) {}

Server::~Server() {}

// Private --------------------------------------------------------------------

// ----------------------------------------------------------------------------

Listener &		Server::listener() {
	return _listener;
}

void			Server::addClient(int fd) {
	_clients[fd].initClient(fd, this);
}

Client	&		Server::getClient(int fd) {
	return _clients[fd];
}