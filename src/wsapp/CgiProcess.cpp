#include "CgiProcess.hpp"

CgiProcess::CgiProcess()
	: _server_port(-1), _child_pid(-1), _rinfo()
{
}

void CgiProcess::init(ParseResult const &pars_res, int server_port, const remote_info &rinfo)
{
	_pars_res = pars_res;
	_server_port = server_port;
	_rinfo = rinfo;
}

void CgiProcess::start()
{
	if (!_parent_to_child.create() || !_child_to_parent.create())
	{
		throw Pipe::PipeException();
	}

	pid_t pid = fork();
	if (pid < 0)
	{
		throw ForkException();
	}

	// Child process (Standard Blocking I/O)
	if (pid == 0)
	{
		if (dup2(_parent_to_child.getReadFd(), STDIN_FILENO) == -1 ||
			dup2(_child_to_parent.getWriteFd(), STDOUT_FILENO) == -1)
		{
			throw ChildProcessException();
		}

		_parent_to_child.closeAll();
		_child_to_parent.closeAll();

		// TODO: resolve the CGI script path using server's configuration
		// and the incoming request
		const char *argv[] = {"/usr/bin/python3", "scripts/cgi.py", NULL};

		std::vector<const char *> envp = buildEnvp();
		execve(argv[0], const_cast<char *const *>(argv), const_cast<char *const *>(envp.data()));

		throw ChildProcessException();
	}
	// Parent process (Non-blocking I/O for epoll)
	else
	{
		_parent_to_child.closeRead();
		_child_to_parent.closeWrite();

		// 1. Set ONLY the parent's read end to non-blocking for epoll
		_child_to_parent.setNonBlockingRead();

		// 2. If parent has a request body to send to CGI stdin, set non-blocking write:
		// _parent_to_child.setNonBlockingWrite();

		// 3. Store child pid for non-blocking waitpid(pid, status, WNOHANG)
		_child_pid = pid;
	}
}

std::vector<const char *> CgiProcess::buildEnvp()
{
	populateCgiVars();

	std::vector<const char *> envp;
	for (size_t i = 0; i < _cgi_vars.size(); ++i)
	{
		envp.push_back(_cgi_vars[i].c_str());
	}
	envp.push_back(NULL);

	return envp;
}

void CgiProcess::populateCgiVars()
{
	std::string host = _pars_res.request.host;
	std::string server_name = host;
	std::string server_port = Utils::to_string(_server_port);

	size_t colon_pos = host.find(':');
	if (colon_pos != std::string::npos)
	{
		server_name = host.substr(0, colon_pos);
		server_port = host.substr(colon_pos + 1);
	}

	// Mandatory
	_cgi_vars.push_back("GATEWAY_INTERFACE=CGI/1.1");
	_cgi_vars.push_back("REMOTE_ADDR=" + _rinfo.addr);
	_cgi_vars.push_back("REMOTE_PORT=" + Utils::to_string(_rinfo.port));
	_cgi_vars.push_back("REQUEST_METHOD=" + _pars_res.request.method);
	_cgi_vars.push_back("SCRIPT_NAME=" + _pars_res.request.path);
	_cgi_vars.push_back("REDIRECT_STATUS=200");
	_cgi_vars.push_back("SCRIPT_FILENAME=" + _pars_res.request.raw_path);
	_cgi_vars.push_back("SERVER_NAME=" + server_name);
	_cgi_vars.push_back("SERVER_PORT=" + server_port);
	_cgi_vars.push_back("SERVER_PROTOCOL=HTTP/1.0");
	_cgi_vars.push_back("SERVER_SOFTWARE=webserv");

	// optional
	_cgi_vars.push_back("CONTENT_LENGTH=" + Utils::to_string(_pars_res.request.content_length));
	std::string content_type = _pars_res.request.getHeader("Content-Type");
	if (!content_type.empty())
	{
		_cgi_vars.push_back("CONTENT_TYPE=" + content_type);
	}
	std::string query_string = _pars_res.request.query;
	if (!query_string.empty())
	{
		_cgi_vars.push_back("QUERY_STRING=" + query_string);
	}

	for (std::map<std::string, std::string>::const_iterator it = _pars_res.request.headers.begin(); it != _pars_res.request.headers.end(); it++)
	{
		std::string cgi_var_key = "HTTP_" + it->first;

		for (size_t idx = 0; idx < cgi_var_key.length(); idx++)
		{
			if (cgi_var_key[idx] == '-')
				cgi_var_key[idx] = '_';
			cgi_var_key[idx] = std::toupper(cgi_var_key[idx]);
		}
		_cgi_vars.push_back(cgi_var_key + "=" + it->second);
	}
}

int CgiProcess::getCgiReadFd() const
{
	return _child_to_parent.getReadFd();
}

int CgiProcess::getCgiWriteFd() const
{
	return _parent_to_child.getWriteFd();
}

const char *CgiProcess::ForkException::what() const throw()
{
	return "CgiProcess::ForkException.";
}

const char *CgiProcess::ChildProcessException::what() const throw()
{
	return "CgiProcess::ChildProcessException.";
}
