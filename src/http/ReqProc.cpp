#include "ReqProc.hpp"

#include <sys/stat.h>
#include <algorithm>

ReqProc::result		ReqProc::process(ParsedRequest const &req, ServerDirective const &servDir)
{
	LocationDirective const &	locDir = *ServerDirective::matchLocation(servDir.getLocations(), req.path);

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
	}

	std::string	body;
	result		result;
	std::string	fsPath;
	ServerDirective::resolveFsPath(servDir, req.path, &locDir, fsPath);

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
					// generate autoindex
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
