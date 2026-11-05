#ifndef REQPROC_HPP
#define REQPROC_HPP

#include "ServerDirective.hpp"
#include "MimeTypes.hpp"
#include "HttpStat.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

class ReqProc
{
public:
	enum status
	{
		RESP_RDY,
		CGI_REQ
	};

	struct result
	{
		status stat;
		std::string resp;

		std::string cgiScript; // resolved script path on disk
		std::string cgiRunner; // interpreter, e.g. /usr/bin/python3

		result() : stat(RESP_RDY) {}
	};

	static result process(ParsedRequest const &req, ServerDirective const &servDir);
};

#endif
