#include "ReqProc.hpp"
#include "MultipartBody.hpp"
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
					//does single additional header require \r\n?
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
				result.resp = HttpResponse::buildError(404, req.path, &servDir);
		}
		else
		{
			struct stat st;
			LOG_DEBUG("Assuming autoindex route");
			LOG_DEBUG("fsPath.c_str(): " << fsPath.c_str());

			// see if this is needed, remove comment if unnecessary:
			// 		if uri does not end with /, force 302 (301, but no lol) to <path> with / appeneded to end 
			
			// stat might cause a problem with alias directives
			// remove this comment if it doesnt and is ok as-is
			if (stat(fsPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
			{
				//if autoindex is enabled
				if (isAutoindexEnabled(locDir, servDir))
					result.resp = HttpResponse::buildAutoindex(fsPath, req.path, &servDir);
				else //directory listing disabled and no index resolved
					result.resp = HttpResponse::buildError(403, req.path, &servDir);
			}
			else //will end up as 404 not found
				result.resp = HttpResponse::buildError(404, req.path, &servDir);
		}
	}
	if (req.method == "POST")
	{
		// uploads are allowed e.g. locDir.getUpload() != NULL
			// {} 403 forbidden (for upload)
		// upload_store provided e.g. UploadDir.getPath()
			// {}
		std::string		upload_store = "html/uploads";

		// try to parse multipart/form-data body
		if (req.body.size() == 0)
		{
			result.resp = HttpResponse::buildError(400, req.path, &servDir);
			return result;
		}
		std::map<std::string, std::string>::const_iterator	cit;
		cit = req.headers.find("content-type");
		if (cit->second.find("multipart/form-data") != 0)
		{
			result.resp = HttpResponse::buildError(415, req.path, &servDir);
			return result;
		}

		MultipartBody	mpb;
		if (!mpb.parse(req.body, cit->second) || mpb.fileCount() == 0)
		{
			result.resp = HttpResponse::buildError(400, req.path, &servDir);
			return result;
		}
		for (size_t i = 0; i < mpb.fileCount(); i++)
		{
			MultipartBody::part		p = mpb.filePart(i);
			if (!Utils::saveFile(upload_store, p.fdata.find("filename")->second, p.body))
			{
				result.resp = HttpResponse::buildError(500, req.path, &servDir);
				return result;
			}
		}
		result.resp = HttpResponse::build(201, MimeTypes::forExtn(".txt"), "Upload success!");
		return result;
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
