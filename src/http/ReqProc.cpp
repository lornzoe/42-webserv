#include "ReqProc.hpp"
#include "SeshProfile.hpp"
#include "MultipartBody.hpp"
#include "w_logger.hpp"

#include <sys/stat.h>
#include <algorithm>
#include <string>
#include <cstdio>

namespace
{
	bool isAutoindexEnabled(const LocationDirective *locDir, const ServerDirective &servDir)
	{
		if (locDir && locDir->getAutoindex())
			return locDir->getAutoindex()->isEnabled();
		if (servDir.getAutoindex())
			return servDir.getAutoindex()->isEnabled();
		return false;
	}

	bool isMtdAllowed(ParsedRequest const &req, LocationDirective const &locDir)
	{
		const std::vector<std::string> &methods = locDir.getLimitExcept()->getMethods();
		std::vector<std::string>::const_iterator cit = std::find(methods.begin(), methods.end(), req.method);
		if (cit == methods.end())
			return false;
		return true;
	}

	bool getUploadStorePath(const LocationDirective *locDir, const ServerDirective &servDir, std::string &uploadStorePath)
	{
		if (locDir && locDir->getUploadStore())
		{
			uploadStorePath = locDir->getUploadStore()->getPath();
			return true;
		}
		if (servDir.getUploadStore())
		{
			uploadStorePath = servDir.getUploadStore()->getPath();
			return true;
		}
		return false;
	}

	// check if uri has a respective locationdir with a '/' appended to the back
	// e.g. /uploads -> /uploads/
	bool needsLocationSlashRedirect(const std::string& uri, const ServerDirective& servDir)
	{
		std::string slashed;

		if (uri.empty() || uri[uri.size() - 1] == '/')
			return false;
		else 
			slashed = uri + '/';
		std::vector<const LocationDirective*> locs = servDir.getLocations();
		for (std::size_t i = 0; i < locs.size(); i++)
		{
			if (locs[i]->getPath() == slashed)
				return true;
		}
		return false;
	}
}

ReqProc::result ReqProc::process(ParsedRequest const &req, ServerDirective const &servDir)
{
	const LocationDirective *locDir = ServerDirective::matchLocation(servDir.getLocations(), req.path);

	std::string body;
	result result;
	std::string fsPath;

	// Canonicalize before checking the fallback location's allowed methods.
	if (needsLocationSlashRedirect(req.path, servDir))
	{
		LOG_DEBUG("uri path of " << req.path << " has a slashed location, should send redirect");
		std::string target = req.raw_path + '/';
		if (!req.query.empty())
			target += "?" + req.query;
		// Preserve the method and body when redirecting POST or DELETE.
		result.resp = HttpResponse::build(req.method == "GET" ? 301 : 308, "", "", "Location: " + target);
		return result;
	}

	if (locDir && locDir->getLimitExcept() && !isMtdAllowed(req, *locDir))
		return result.resp = HttpResponse::buildError(405, req.path, &servDir), result;

	if (locDir && locDir->getReturn())
	{
		int code = locDir->getReturn()->getCode();
		if (code >= 300 && code < 400)
		{
			std::string url = locDir->getReturn()->getBody();
			switch (code)
			{
				case 301: // permanent redirect; client may switch to GET
				case 302: // temporary redirect; client may switch to GET
				case 303: // see other
				case 307: // temporary redirect + carryover method
				case 308: // permanent redirect + carryover method
					LOG_DEBUG("Redirecting request to: " << url << " with status code: " << code);
					result.resp = HttpResponse::build(code, "", "", "Location: " + url);
					return result;
				default:
					LOG_WARN("Return directive with code " << code << " is an unsupported code. Returning error 500 instead.");
					result.resp = HttpResponse::buildError(500, "", &servDir);
					return result;
			}
		}
		// non-redirect return
		const std::string& text = locDir->getReturn()->getBody();
		LOG_DEBUG("Return directive responding with status code: " << code);
		if (!text.empty())
			result.resp = HttpResponse::build(code, "text/plain", text); // return code "text here";
		else if (code >= 400)
			result.resp = HttpResponse::buildError(code, req.path, &servDir);	// return 404; (uses error_page)
		else
			result.resp = HttpResponse::build(code, "", "");					// return 200; / return 204;
		return result;
	}

	if (!ServerDirective::resolveFsPath(servDir, req.path, locDir, fsPath))
		return result.resp = HttpResponse::buildError(403, req.path, &servDir), result;

	const CgiDirective *cgi = servDir.findCgi(fsPath);
	if (cgi)
	{
		struct stat st;
		if (stat(fsPath.c_str(), &st) == -1 || !S_ISREG(st.st_mode))
			return result.resp = HttpResponse::buildError(404, req.path, &servDir), result;

		if (req.method != "GET" && req.method != "POST" && req.method != "DELETE")
			return result.resp = HttpResponse::buildError(405, req.path, &servDir), result;

		result.stat = CGI_REQ;
		result.cgiScript = fsPath;
		result.cgiRunner = cgi->getRunner();
		return result;
	}

	if (req.method == "GET")
	{
		ServerDirective::ResourcePath rsrc_path = servDir.getResource(req.path);
		if (rsrc_path.first == true)
		{
			if (Utils::readFile(rsrc_path.second, body))
			{
				if (req.path == "/profile/")
					SeshProfile::dynamic_fill(body, *req.sdata);
				result.resp = HttpResponse::build(200, MimeTypes::forPath(rsrc_path.second), body);
			}
			else
				result.resp = HttpResponse::buildError(404, req.path, &servDir);
		}
		else
		{
			struct stat st;
			LOG_DEBUG("Assuming autoindex route");
			LOG_DEBUG("fsPath.c_str(): " << fsPath.c_str());

			// stat might cause a problem with alias directives
			// remove this comment if it doesnt and is ok as-is
			if (stat(fsPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
			{
				// if autoindex is enabled
				if (isAutoindexEnabled(locDir, servDir))
					result.resp = HttpResponse::buildAutoindex(fsPath, req.path, &servDir);
				else // directory listing disabled and no index resolved
					result.resp = HttpResponse::buildError(403, req.path, &servDir);
			}
			else // will end up as 404 not found
				result.resp = HttpResponse::buildError(404, req.path, &servDir);
		}
	}
	if (req.method == "POST")
	{
		if (req.path == "/profile/")
		{
			SeshProfile::post_update(req.body, *req.sdata);
			result.resp = HttpResponse::build(303, "text/html", "");
			result.resp.insert(result.resp.find(HEADER_END), "\r\nLocation: /profile/");
			return result;
		}
		// uploads are allowed e.g. locDir.getUpload() != NULL -- 403
		// upload_store provided e.g. UploadDir.getPath() -- proceed as normal
		// no upload_store in location or server means uploads are not allowed
		std::string upload_store;
		if (!getUploadStorePath(locDir, servDir, upload_store))
		{
			result.resp = HttpResponse::buildError(403, req.path, &servDir);
			return result;
		}

		// try to parse multipart/form-data body
		if (req.body.size() == 0)
		{
			result.resp = HttpResponse::buildError(400, req.path, &servDir);
			return result;
		}
		std::map<std::string, std::string>::const_iterator cit;
		cit = req.headers.find("content-type");
		if (cit == req.headers.end() || cit->second.find("multipart/form-data") != 0)
		{
			result.resp = HttpResponse::buildError(415, req.path, &servDir);
			return result;
		}

		MultipartBody mpb;
		if (!mpb.parse(req.body, cit->second) || mpb.fileCount() == 0)
		{
			result.resp = HttpResponse::buildError(400, req.path, &servDir);
			return result;
		}
		for (size_t i = 0; i < mpb.fileCount(); i++)
		{
			MultipartBody::part p = mpb.filePart(i);
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
		struct stat st;
		int code;

		if (stat(fsPath.c_str(), &st) == -1)
			code = 204;
		else if (S_ISREG(st.st_mode))
			code = std::remove(fsPath.c_str()) == 0 ? 200 : 403;
		else
			code = 403;

		if (code >= 200 && code <= 299)
			result.resp = HttpResponse::build(code, MimeTypes::forExtn(".txt"), "");
		else
			result.resp = HttpResponse::buildError(code, req.path, &servDir);
	}

	return result;
}
