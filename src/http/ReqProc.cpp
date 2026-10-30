#include "ReqProc.hpp"
#include "MultipartBody.hpp"
#include "w_logger.hpp"

#include <sys/stat.h>
#include <algorithm>
#include <string>
#include <cstdio>

namespace {
	bool isAutoindexEnabled(const LocationDirective* locDir, const ServerDirective& servDir)
	{
		if (locDir && locDir->getAutoindex())
			return locDir->getAutoindex()->isEnabled();
		if (servDir.getAutoindex())
			return servDir.getAutoindex()->isEnabled();
		return false;
	}

	bool	isMtdAllowed(ParsedRequest const &req, LocationDirective const &locDir)
	{
		const std::vector<std::string> &	methods = locDir.getLimitExcept()->getMethods();
		std::vector<std::string>::const_iterator cit = std::find(methods.begin(), methods.end(), req.method);
		if (cit == methods.end())
			return false;
		return true;
	}

	bool getUploadStorePath(const LocationDirective* locDir, const ServerDirective& servDir, std::string& uploadStorePath)
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

	// For session profile demo

	int	hex_value(char c)
	{
		if (c >= '0' && c <= '9')
			return c - '0';
		if (c >= 'a' && c <= 'f')
			return c - 'a' + 10;
		if (c >= 'A' && c <= 'F')
			return c - 'A' + 10;
		return -1;
	}

	std::string	url_decode(std::string const &str)
	{
		std::string	out;
		for (size_t i = 0; i < str.size(); ++i)
		{
			if (str[i] == '+')
				out += ' ';
			else if (str[i] == '%' && i + 2 < str.size())
			{
				int hi = hex_value(str[i + 1]);
				int lo = hex_value(str[i + 2]);

				if (hi != -1 && lo != -1)
				{
					out += static_cast<char>(hi * 16 + lo);
					i += 2;
				}
				else
					out += str[i];
			}
			else
				out += str[i];
		}
		return out;
	}

	std::string	html_escape(std::string const &str)
	{
		std::string	out;
		for (size_t i = 0; i < str.size(); ++i)
		{
			switch (str[i])
			{
				case '&':	out += "&amp;";	break;
				case '<':	out += "&lt;";	break;
				case '>':	out += "&gt;";	break;
				case '"':	out += "&quot;";	break;
				case '\'':	out += "&#39;";	break;
				default:	out += str[i];
			}
		}
		return out;
	}

	void	replace_all(std::string &body, std::string const &token, std::string const &value)
	{
		size_t	pos = 0;

		while ((pos = body.find(token, pos)) != std::string::npos)
		{
			body.replace(pos, token.size(), value);
			pos += value.size();
		}
	}

	std::map<std::string, std::string>	make_foods()
	{
		std::map<std::string, std::string> foods;
		foods["Pizza"] = "pizza.jpeg";
		foods["Salad"] = "salad.jpeg";
		foods["Burger"] = "burger.jpeg";
		foods["Sushi"] = "sushi.jpeg";
		foods["Pasta"] = "pasta.webp";
		foods["Chicken Rice"] = "chicken_rice.jpeg";
		foods["Steak"] = "steak.jpeg";
		foods["Tacos"] = "tacos.jpeg";
		foods["Ice Cream"] = "ice_cream.jpeg";
		foods["Durian"] = "durian.jpeg";
		return foods;
	}

	std::map<std::string, std::string> const	&food_map()
	{
		static const std::map<std::string, std::string> foods = make_foods();
		return foods;
	}

	void	update_sesh_profile(ParsedRequest const &req)
	{
		sesh_data	&sesh = *req.sdata;

		std::vector<std::string> fields =
			Utils::ft_split(req.body, "&", true);

		for (std::vector<std::string>::iterator it = fields.begin();
			it != fields.end(); ++it)
		{
			Utils::kvpair kv;

			if (!Utils::split_pair(*it, '=', kv))
				continue;

			std::string key = url_decode(kv.first);
			std::string val = url_decode(kv.second);

			if (key == "name")
			{
				// Empty means "leave unchanged".
				if (!val.empty())
					sesh.name = val;
			}
			else if (key == "fav_food")
			{
				// Empty means "leave unchanged".
				if (!val.empty()
					&& food_map().find(val) != food_map().end())
				{
					sesh.fav_food = val;
				}
			}
		}
		sesh.profile_updated = true;
	}

	void	fill_sesh_profile(std::string &body, ParsedRequest const &req)
	{
		sesh_data	&sesh = *req.sdata;
		sesh.profile_visits++;
		std::map<std::string, std::string> const &foods =
			food_map();

		// Template replacement: Food options -------------------------
		std::stringstream	options;
		options
			<< "<option value=\"\" selected>"
			<< "Choose one..."
			<< "</option>\n";

		std::map<std::string, std::string>::const_iterator it;

		for (it = foods.begin(); it != foods.end(); ++it)
		{
			options
				<< "<option value=\""
				<< html_escape(it->first)
				<< "\">"
				<< html_escape(it->first)
				<< "</option>\n";
		}

		// Template replacement: Fav food -----------------------------
		std::string	image;

		it = foods.find(sesh.fav_food);
		if (it != foods.end())
		{
			image =
				"<img class=\"food-image\" src=\"/profile_pics/"
				+ it->second
				+ "\" alt=\""
				+ html_escape(it->first)
				+ "\">";
		}

		// Template replacement: Profile visits -----------------------
		std::stringstream	visits;
		visits << sesh.profile_visits;

		// Template replacement: Update message -----------------------
		std::string	updated_message;

		if (sesh.profile_updated)
		{
			updated_message = "<p class=\"updated\">Profile updated!</p>";
			sesh.profile_updated = false;
		}

		// Fill templates
		replace_all(body, "{{SID}}", html_escape(sesh.sid));
		replace_all(body, "{{NAME}}", html_escape(sesh.name));
		replace_all(body, "{{FAV_FOOD}}", sesh.fav_food.empty() ? "Not selected" : html_escape(sesh.fav_food));
		replace_all(body, "{{PROFILE_VISITS}}", visits.str());
		replace_all(body, "{{UPDATED_MESSAGE}}", updated_message);
		replace_all(body, "{{FOOD_IMAGE}}", image);
		replace_all(body, "{{FOOD_OPTIONS}}", options.str());
	}
}

ReqProc::result		ReqProc::process(ParsedRequest const &req, ServerDirective const &servDir)
{
	const LocationDirective*	locDir = ServerDirective::matchLocation(servDir.getLocations(), req.path);
	
	std::string	body;
	result		result;
	std::string	fsPath;

	if (locDir && locDir->getLimitExcept() && !isMtdAllowed(req, *locDir))
		return result.resp = HttpResponse::buildError(405, req.path, &servDir), result;

	if (locDir && locDir->getReturn())
	{
		//redirect
			// resp avail imme diately
			// resp code: rdir.getCode()
			// resp location: rdir.getBody()
		int code = locDir->getReturn()->getCode();
		if (code >= 300 && code < 400)
		{
			std::string url = locDir->getReturn()->getBody();
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

	if (!ServerDirective::resolveFsPath(servDir, req.path, locDir, fsPath))
		return result.resp = HttpResponse::buildError(403, req.path, &servDir), result;

	// For CGI
		// if fsPath ends in recognized CGI extension
		// and locDir has CGI enabled
			// note execution context info: script path and interpreter etc.
			// defer resp to CGI: result.stat = CGI_REQ

	if (req.method == "GET")
	{
		ServerDirective::ResourcePath	rsrc_path = servDir.getResource(req.path);
		if (rsrc_path.first == true)
		{
			if (Utils::readFile(rsrc_path.second, body))
			{
				if (req.path == "/profile")
					fill_sesh_profile(body, req);
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
		if (req.path == "/profile")
		{
			update_sesh_profile(req);
			result.resp = HttpResponse::build(303, "text/html", "");
			result.resp.insert(result.resp.find(HEADER_END), "\r\nLocation: /profile");
			return result;
		}
		// uploads are allowed e.g. locDir.getUpload() != NULL -- 403
		// upload_store provided e.g. UploadDir.getPath() -- proceed as normal
		// no upload_store in location or server means uploads are not allowed
		std::string		upload_store;
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
		struct stat st;
		int			code;

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
