#ifndef REQPROC_HPP
# define REQPROC_HPP

# include "ServerDirective.hpp"
# include "MimeTypes.hpp"
# include "HttpStat.hpp"
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"

class ReqProc
{
private:
	static bool			isMtdAllowed(ParsedRequest const &req, LocationDirective const &locDir);

public:

	enum status {
		RESP_RDY,
		CGI_REQ
	};

	struct result {
		status			stat;
		std::string		resp;

		result() : stat(RESP_RDY) {}
	};

	static result	process(ParsedRequest const &req, ServerDirective const &servDir);
};

#endif
