#include "Client.hpp"
#include "w_eventCtx.hpp"
#include "w_utils.hpp"

// OCF ------------------------------------------------------------------------

Client::Client() : _server(NULL), _fd(-1) {}

Client::~Client() {
	wutils::safeClose(_fd);
}

// Private --------------------------------------------------------------------

// ----------------------------------------------------------------------------

void	Client::initClient(Server &server, int fd) {
	if (_fd != -1)
	{
		wutils::safeClose(_fd);
		// and reset internal state
	}
	_fd = fd;
	_server = &server;

	_eCtx.fd = _fd;
	_eCtx.type = SCK_CLIENT;
	_eCtx.owner = this;
}
