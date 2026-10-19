#ifndef CGIPROCESS_H
#define CGIPROCESS_H

#include <vector>

#include "Pipe.hpp"
#include "HttpRequest.hpp"

struct remote_info
{
	std::string addr;
	int port;
};

class CgiProcess
{
public:
	CgiProcess();

	void init(ParseResult const &pars_res, int server_port, const remote_info &rinfo);
	void start();
	int getCgiReadFd() const;
	int getCgiWriteFd() const;

	class ForkException : public std::exception
	{
	public:
		virtual const char *what() const throw();
	};

	class ChildProcessException : public std::exception
	{
	public:
		virtual const char *what() const throw();
	};

private:
	int _server_port;
	Pipe _parent_to_child;
	Pipe _child_to_parent;
	pid_t _child_pid;
	ParseResult _pars_res;
	std::vector<std::string> _cgi_vars;
	remote_info _rinfo;

	void populateCgiVars();
	std::vector<const char *> buildEnvp();
};

#endif
