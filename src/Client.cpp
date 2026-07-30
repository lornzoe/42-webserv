#include "Client.hpp"
#include "EventCtx.hpp"
#include "WUtils.hpp"

// OCF ------------------------------------------------------------------------

Client::Client() : _fd(-1) {}

Client::~Client() {
	WUtils::safeClose(_fd);
}

Client::Client(Client const &other) : _fd(-1) { (void)other; }

Client &	Client::operator=(Client const &other)
{
	if (this == &other)
		return *this;
	_fd = -1;
	return *this;
}

// Private --------------------------------------------------------------------

// ----------------------------------------------------------------------------

void	Client::initClient(int fd, Server const * server) {
	if (_fd != -1)
	{
		WUtils::safeClose(_fd);
		// and reset internal state
	}
	_fd = fd;
	_server = server;

	_ctx.fd = _fd;
	_ctx.type = SCK_CLIENT;
	_ctx.owner = this;
}

int				Client::fd() {
	return _fd;
}

EventCtx *		Client::ctx() {
	return &_ctx;
}
