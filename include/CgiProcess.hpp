#ifndef CGIPROCESS_H
#define CGIPROCESS_H

#include <vector>

#include "Pipe.hpp"
#include "HttpRequest.hpp"
#include "w_eventCtx.hpp"

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
	void closeCgiReadFd();
	int getCgiWriteFd() const;
	void closeCgiWriteFd();

	eventCtx &get_in_ectx();
	eventCtx &get_out_ectx();

	void appendOutbox(std::string const &data);
	std::string const &getOutbox() const;

	std::string const &getRequestBody() const;
	size_t &getBodyWriteOffset();

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

	eventCtx _in_eCtx;
	eventCtx _out_eCtx;

	std::string _requestBody;
	size_t _bodyWriteOffset;

	std::string _cgi_outbox;

	void populateCgiVars();
	std::vector<const char *> buildEnvp();
};

#endif
