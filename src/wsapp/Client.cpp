#include "Client.hpp"
#include "Server.hpp"
#include "Utils.hpp"
#include "w_eventCtx.hpp"
#include "w_logger.hpp"

#include <sys/socket.h>
#include <iostream>

// OCF ------------------------------------------------------------------------

Client::Client()
	: _server(NULL),
	  _fd(-1),
	  _eCtx(eventCtx()),
	  _status(0),
	  _outPend(0),
	  _outCursor(0),
	  _last_activity_time(time(NULL)),
	  _sdata(NULL)
{
}

Client::Client(Client const &other)
	: _server(NULL),
	  _fd(-1),
	  _eCtx(eventCtx()),
	  _status(0),
	  _outPend(0),
	  _outCursor(0),
	  _last_activity_time(time(NULL)),
	  _sdata(NULL)
{
	(void)other;
}

Client &Client::operator=(Client const &other)
{
	if (this == &other)
		return *this;
	_server = NULL;
	_fd = -1;
	_eCtx = eventCtx();
	_status = 0;
	_outPend = 0;
	_outCursor = 0;
	_last_activity_time = time(NULL);
	_sdata = NULL;
	return *this;
}

Client::~Client()
{
	Utils::safeClose(_fd);
}

// Private --------------------------------------------------------------------

// ----------------------------------------------------------------------------

void Client::initClient(Server &server, int fd)
{
	if (_fd != -1)
	{
		Utils::safeClose(_fd);
		// and reset internal state
	}
	_server = &server;
	_fd = fd;

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

ReqProc::status Client::process_request(ParseResult &pars_res)
{
	std::map<std::string, std::string>::const_iterator cit;
	cit = pars_res.request.headers.find("connection");
	if (cit != pars_res.request.headers.end() && cit->second == "close")
		addStat(CLOSING);

	//Cookies & Session ---------------------------
	std::string sid;
	SeshCtl &	sctl = server().seshctl();
	sesh_data *sdata;

	//Confirm SID exists and create or retrieve
	cit = pars_res.request.headers.find("cookie");
	if (cit == pars_res.request.headers.end() 
		|| (sid = SeshCtl::get_header_sid(cit->second)) == ""
		|| (sdata = sctl.retrieve(sid)) == NULL)
	{
		addStat(NEWSESH);
		_sdata = &sctl.create();
	}
	else
		_sdata = sdata;
	pars_res.request.sdata = _sdata;
	//Cookies & Session ---------------------------

	ReqProc::result resp_res = ReqProc::process(pars_res.request, servDir());
	comsume_inbox(pars_res.consumed);
	if (resp_res.stat == ReqProc::RESP_RDY)
	{
		if (isStat(CLOSING))
			resp_res.resp.insert(resp_res.resp.find(HEADER_END), "\r\nConnection: close");
		if (isStat(NEWSESH))
		{
			std::string set_cookie_hdr = SeshCtl::set_cookie_header(_sdata->sid);
			resp_res.resp.insert(resp_res.resp.find(HEADER_END), "\r\n" + set_cookie_hdr);
			rmStat(NEWSESH);
		}
		send_response(resp_res.resp);
		_sdata = NULL;
	}
	else
	{
		_sdata->activeCGI++;
		// Some embedded CGI object?
		// save pars_res body, set up child with meta vars & pipes
		// have api to
		// write body to cgi_write
		// read cgi_out into cgi_res buf
		// check progress
		// etc.
		// add stat WAIT_CGI
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
