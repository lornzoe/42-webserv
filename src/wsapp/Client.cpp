#include "Client.hpp"
#include "Server.hpp"
#include "Utils.hpp"
#include "w_eventCtx.hpp"
#include "w_logger.hpp"
#include <Pipe.hpp>

#include <iostream>
#include <sys/socket.h>
#include <sys/wait.h>

// OCF ------------------------------------------------------------------------

Client::Client()
	: _server(NULL),
	  _fd(-1),
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
		// Some embedded CGI object?
		// save pars_res body, set up child with meta vars & pipes
		// have api to
		// write body to cgi_write
		// read cgi_out into cgi_res buf
		// check progress
		// etc.
		// add stat WAIT_CGI

		// TODO: Create a CgiProcess class and let client owns it
		Pipe parent_to_child;
		Pipe child_to_parent;

		// TODO: Throw exception
		if (!parent_to_child.create() || !child_to_parent.create())
		{
			return resp_res.stat;
		}

		pid_t pid = fork();
		// TODO: Throw exception
		if (pid < 0)
		{
			std::exit(1);
		}

		// Child process
		if (pid == 0)
		{
			// TODO: Throw exception
			if (dup2(parent_to_child.getReadFd(), STDIN_FILENO) == -1 ||
				dup2(child_to_parent.getWriteFd(), STDOUT_FILENO) == -1)
			{
				_exit(1);
			}

			parent_to_child.closeAll();
			child_to_parent.closeAll();

			// TODO: resolve the CGI script path using server's configuration
			// and the incoming request
			char *argv[3];
			argv[0] = const_cast<char *>("/usr/bin/python3");
			argv[1] = const_cast<char *>("scripts/cgi.py");
			argv[2] = NULL;

			// TODO:
			char *envp[3];
			envp[0] = const_cast<char *>("MY_VAR1=env1");
			envp[1] = const_cast<char *>("MY_VAR2=env2");
			envp[2] = NULL;

			execve(argv[0], argv, envp);

			_exit(1);
		}
		// Parent process
		else
		{
			parent_to_child.closeRead();
			child_to_parent.closeWrite();

			// TODO: Write to parent_to_child_pipes[1]
			parent_to_child.closeWrite();

			// Read the CGI script's output.
			std::string cgi_output;
			char buffer[1024];
			ssize_t bytes_read;

			while ((bytes_read = read(
						child_to_parent.getReadFd(),
						buffer,
						sizeof(buffer))) > 0)
			{
				cgi_output.append(buffer, bytes_read);
			}

			if (bytes_read < 0)
			{
				std::cerr << "Error reading CGI output" << std::endl;
			}

			child_to_parent.closeRead();

			LOG_DEBUG(cgi_output);

			std::string::size_type header_end = cgi_output.find("\r\n\r\n");

			if (header_end == std::string::npos)
			{
				// Invalid or incomplete CGI response.
			}
			std::string cgi_headers = cgi_output.substr(0, header_end);
			std::string cgi_body = cgi_output.substr(header_end + 4);

			LOG_DEBUG(cgi_headers);
			LOG_DEBUG(cgi_body);

			// TODO: this needs to be changed as it is blocking
			int status;
			waitpid(pid, &status, 0);

			if (WIFEXITED(status))
			{
				std::cout << "CGI exit code: "
						  << WEXITSTATUS(status) << std::endl;
			}
		}
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
