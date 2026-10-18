#ifndef CGIPROCESS_H
#define CGIPROCESS_H

#include <vector>

#include <Pipe.hpp>
#include "HttpRequest.hpp"

class CgiProcess
{
public:
	CgiProcess();

	void init(ParseResult const &pars_res);
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
	Pipe _parent_to_child;
	Pipe _child_to_parent;
	pid_t _child_pid;
	ParseResult _pars_res;
	std::vector<std::string> _cgi_vars;

	void populateCgiVars();
	std::vector<const char *> buildEnvp();
};

#endif
