#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>

class ServerDirective;

namespace HttpResponse
{
	std::string build(int code, const std::string &contentType, const std::string &body,
					  const std::string additionalHeaders = "");
	std::string buildError(int code, const std::string &uri, ServerDirective const *servDir);
	std::string buildAutoindex(const std::string &fsDir, const std::string &uriPath, const ServerDirective *servDir);
	std::string defaultErrorBody(int code);
	bool buildCgiResponse(std::string const &cgiOutput, std::string &resp);
	int parseCgiStatus(std::string const &value);
};

#endif
