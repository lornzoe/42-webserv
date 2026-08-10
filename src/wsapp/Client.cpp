#include "Client.hpp"
#include "Server.hpp"
#include "w_eventCtx.hpp"
#include "w_utils.hpp"
#include "w_logger.hpp"

#include <sys/socket.h>
#include <iostream>

// OCF ------------------------------------------------------------------------

Client::Client()
: _server(NULL), _fd(-1), _eCtx(eventCtx()), _status(0), _outPend(0), _outCursor(0) {}

Client::Client(Client const & other)
: _server(NULL), _fd(-1), _eCtx(eventCtx()), _status(0), _outPend(0), _outCursor(0)
{ (void)other; }

Client &	Client::operator=(Client const &other)
{
	if (this == &other)
		return *this;
	_server = NULL;
	_fd = -1;
	_eCtx = eventCtx();
	_status = 0;
	_outPend = 0;
	_outCursor = 0;
	return *this;
}

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
	_server = &server;
	_fd = fd;

	_eCtx.fd = _fd;
	_eCtx.type = SCK_CLIENT;
	_eCtx.owner = this;
}

ServerDirective const &	Client::servDir() const {
	return _server->servDir();
}

ssize_t		Client::recv1()
{
	char	buf[4096];
	ssize_t bytesRd = recv(_fd, buf, 1000, 0);

	if (bytesRd > 0)
	{
		_inbox.append(buf, bytesRd);
		if (bytesRd < 4096) buf[bytesRd] = '\0';
		std::string	request = buf;
		std::cout << "Message from client: \n" << request << std::endl;
	}

	return bytesRd;
}

void	Client::tmp_req_for_resp(ssize_t req_offset, std::string const &resp)
{
	if (req_offset == -1 || static_cast<size_t>(req_offset) >= _inbox.size())
		_inbox.erase();
	else
		_inbox = _inbox.substr(req_offset);
	_outBox.append(resp);
	_outPend = _outBox.size();
	_status |= SENDING;
}

ssize_t		Client::send1()
{
	if (_outPend == 0)
		return 0;

	ssize_t bytesSent = send(_fd, &_outBox[_outCursor], _outPend, MSG_NOSIGNAL);
	if (bytesSent > 0)
	{
		_outPend -= bytesSent;
		_outCursor += bytesSent;
	}
	if (_outPend == 0)
	{
		_outBox.erase();
		_outCursor = 0;
		_status &= ~SENDING;
	}
	return bytesSent;
}
