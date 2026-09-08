#include "ReqProc.hpp"
#include "w_logger.hpp"

#include <sys/stat.h>
#include <algorithm>
#include <string>

namespace {
	bool isAutoindexEnabled(const LocationDirective& locDir, const ServerDirective& servDir)
	{
		if (locDir.getAutoindex())
			return locDir.getAutoindex()->isEnabled();
		if (servDir.getAutoindex())
			return servDir.getAutoindex()->isEnabled();
		return false;
	}
}

ReqProc::result		ReqProc::process(ParsedRequest const &req, ServerDirective const &servDir)
{
	LocationDirective const &	locDir = *ServerDirective::matchLocation(servDir.getLocations(), req.path);
	
	std::string	body;
	result		result;
	std::string	fsPath;
	ServerDirective::resolveFsPath(servDir, req.path, &locDir, fsPath);

	if (locDir.getLimitExcept() && !isMtdAllowed(req, locDir))
	{
		//method not allowed
			// resp avail immediately
			// statusMap[405] = "Method Not Allowed"
	}
	if (locDir.getReturn())
	{

		//redirect
			// resp avail immediately
			// resp code: rdir.getCode()
			// resp location: rdir.getBody()
		int code = locDir.getReturn()->getCode();
		if (code >= 300 && code < 400)
		{
			std::string url = locDir.getReturn()->getBody();
			switch(code)
			{
				case 301:
				case 302:
					LOG_DEBUG("Redirecting request to: " << url << " with status code: " << code);
					result.resp = HttpResponse::build(code, "", "", "Location: " + url + "\r\n");
					return result;
				case 303:
				case 304:
				case 307:
				case 308:
					break;
				default:
					LOG_WARN("Return directive with code " << code << " is not a valid redirection code. Returning error 500 instead.");
					result.resp = HttpResponse::buildError(500, "", &servDir);
					return result;
			}
		}
	}

	// For CGI
		// if fsPath ends in recognized CGI extension
		// and locDir has CGI enabled
			// defer resp to CGI
			// GET CGI: args in query
			// POST CGI: args in body

	if (req.method == "GET")
	{
		ServerDirective::ResourcePath	rsrc_path = servDir.getResource(req.path);
		if (rsrc_path.first == true)
		{
			if (Utils::readFile(rsrc_path.second, body))
				result.resp = HttpResponse::build(200, MimeTypes::forPath(rsrc_path.second), body);
			else
				result.resp = HttpResponse::buildError(404, rsrc_path.second, &servDir);
		}
		else
		{
			struct stat st;
			if (stat(fsPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
			{
				//if autoindex is enabled
				if (isAutoindexEnabled(locDir, servDir))
				{
					// generate autoindex
					// test with custom error page
					LOG_DEBUG("Autoindex enabled for directory: " << fsPath);
					result.resp = HttpResponse::buildError(200, rsrc_path.second, &servDir);
				}
				//else directory listing has been disabled: no autoindex / resource (incld index)
					// appropriate error
			}
			else //will end up as 404 not found
				result.resp = HttpResponse::buildError(404, rsrc_path.second, &servDir);
		}
	}
	if (req.method == "POST")
	{
		//handle POSTs
			//uploads
		
	}
	if (req.method == "DELETE")
	{
		//handle DELETEs
	}

	return result;
}

bool	ReqProc::isMtdAllowed(ParsedRequest const &req, LocationDirective const &locDir)
{
	const std::vector<std::string> &	methods = locDir.getLimitExcept()->getMethods();
	std::vector<std::string>::const_iterator cit = std::find(methods.begin(), methods.end(), req.method);
	if (cit != methods.end())
		return false;
	return true;
}
