#include "Client.hpp"
#include "Server.hpp"
#include "Utils.hpp"
#include "w_eventCtx.hpp"
#include "w_logger.hpp"

#include <iostream>
#include <sys/socket.h>
#include <sys/wait.h>

// OCF ------------------------------------------------------------------------

Client::Client()
	: _server(NULL),
	  _fd(-1),
	  _rinfo(remote_info()),
	  _eCtx(eventCtx()),
	  _status(0),
	  _outPend(0),
	  _outCursor(0),
	  _last_activity_time(time(NULL))
{
}

Client::Client(Client const &other)
	: _server(NULL),
	  _fd(-1),
	  _rinfo(remote_info()),
	  _eCtx(eventCtx()),
	  _status(0),
	  _outPend(0),
	  _outCursor(0),
	  _last_activity_time(time(NULL))
{
	(void)other;
}

Client &Client::operator=(Client const &other)
{
	if (this == &other)
		return *this;
	_server = NULL;
	_fd = -1;
	_rinfo = remote_info();
	_eCtx = eventCtx();
	_status = 0;
	_outPend = 0;
	_outCursor = 0;
	_last_activity_time = time(NULL);
	return *this;
}

Client::~Client()
{
	Utils::safeClose(_fd);
}

// Private --------------------------------------------------------------------

// ----------------------------------------------------------------------------

void Client::initClient(Server &server, int fd, const remote_info &rinfo)
{
	if (_fd != -1)
	{
		Utils::safeClose(_fd);
		// and reset internal state
	}
	_server = &server;
	_fd = fd;
	_rinfo = rinfo;

	_eCtx.fd = _fd;
	_eCtx.type = SCK_CLIENT;
	_eCtx.owner = this;
}

ServerDirective const &Client::servDir() const
{
	return _server->servDir();
}

ssize_t Client::recv1()
{
	char buf[4096];
	ssize_t bytesRd = recv(_fd, buf, 1000, 0);

	if (bytesRd > 0)
	{
		_inbox.append(buf, bytesRd);
		if (bytesRd < 4096)
			buf[bytesRd] = '\0';
	}

	return bytesRd;
}

ReqProc::status Client::process_request(ParseResult const &pars_res)
{
	std::map<std::string, std::string>::const_iterator cit;
	cit = pars_res.request.headers.find("connection");
	if (cit != pars_res.request.headers.end() && cit->second == "close")
		addStat(CLOSING);

	ReqProc::result resp_res = ReqProc::process(pars_res.request, servDir());
	comsume_inbox(pars_res.consumed);
	if (resp_res.stat == ReqProc::RESP_RDY)
	{
		if (isStat(CLOSING))
			resp_res.resp.insert(resp_res.resp.find(HEADER_END), "\r\nConnection: close");
		send_response(resp_res.resp);
	}
	else
	{
		_cgi.init(pars_res, _server->port(), remote());
		_cgi.start();
	}
	return resp_res.stat;
}

void Client::comsume_inbox(ssize_t req_offset)
{
	if (req_offset == -1 || static_cast<size_t>(req_offset) >= _inbox.size())
		_inbox.erase();
	else
		_inbox = _inbox.substr(req_offset);
}

void Client::send_response(std::string const &resp)
{
	_outBox.append(resp);
	_outPend = _outBox.size();
	_status |= SENDING;
}

ssize_t Client::send1()
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

time_t Client::get_last_activity_time()
{
	return _last_activity_time;
}

void Client::refresh_last_activity_time()
{
	_last_activity_time = time(NULL);
}

int Client::getCgiReadFd() const
{
	return _cgi.getCgiReadFd();
}

int Client::getCgiWriteFd() const
{
	return _cgi.getCgiWriteFd();
}
