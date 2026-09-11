#include "ReqProc.hpp"
#include "w_logger.hpp"

#include <sys/stat.h>
#include <algorithm>
#include <string>
#include <dirent.h>

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

static const char* dirent_type_str(unsigned char t) {
    switch (t) {
        case DT_REG:     return "file";
        case DT_DIR:     return "dir";
        case DT_LNK:     return "symlink";
        case DT_UNKNOWN: return "unknown";
        default:         return "other";
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
				{
					// generate autoindex
					// test with custom error page
					// LOG_DEBUG("Autoindex enabled for directory: " << fsPath);

					DIR *d = opendir(fsPath.c_str());
					if (!d)
					{
						// server error opening directory here. throw error.
						result.resp = HttpResponse::buildError(403, fsPath, &servDir);
						return result;
					}
					struct dirent *entry;
					LOG_DEBUG("entry: d_ino, d_name, d_off, d_reclen, d_type");

					/*
					struct dirent {
						ino_t          d_ino;       // inode number 
						off_t          d_off;       // offset to the next dirent
						unsigned short d_reclen;    // length of this record
						unsigned char  d_type;      // type of file; not supported by all file system types
						char           d_name[256]; // filename
					};
					*/
					
					// need: name, size, date_modified
					// name -- dirent.d_name
					// size -- 
					//
					// how to get the other 2?
					while ((entry = readdir(d)) != NULL) {   // read one entry at a time
					LOG_DEBUG("entry: " << entry->d_ino << ", " << entry->d_name << ", "
										<< entry->d_off << ", " << entry->d_reclen << ", "
										<< dirent_type_str(entry->d_type));
					}
					closedir(d);

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
