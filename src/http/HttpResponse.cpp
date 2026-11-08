#include "HttpResponse.hpp"
#include "HttpStat.hpp"
#include "ServerDirective.hpp"
#include "MimeTypes.hpp"
#include "Utils.hpp"
#include "w_logger.hpp"

#include <sstream>
#include <string>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <ctime>
#include <iostream>

/** Full NON-ERROR http response, provided the relevant parts
 * contentType -- use MimeTypes::forPath(resource_path) to pass appropriate MIME type
 * additionalHeaders -- formatted in bulk, separated by CRLF if >1
 * body -- usually the resource (html, image, text file etc), unless to be generated e.g. defaultErrorBody
 */
std::string HttpResponse::build(int code, const std::string &contentType, const std::string &body, const std::string additionalHeaders)
{
	// 1xx and 204 has no body and no Content-Length;
	// 304 has no body
	bool noBody = (code >= 100 && code < 200) || code == 204 || code == 304;
	bool noLength = (code >= 100 && code < 200) || code == 204;

	// build the header first
	std::stringstream ss;
	ss << "HTTP/1.1 " << code << " " << HttpStat::getReason(code) << CRLF;
	if (!contentType.empty())
		ss << "Content-Type: " << contentType << CRLF;
	if (!additionalHeaders.empty())
		ss << additionalHeaders << CRLF;
	if (!noLength)
		ss << "Content-Length: " << (noBody ? 0 : body.size()) << HEADER_END;
	else
		ss << CRLF; // second CRLF to signify header_end

	// build the body
	if (!noBody)
		ss << body;

	return ss.str();
}

// Looks up a configured custom error page for the given uri/code and, when one
// exists and can be read, fills `out` with a ready-to-send response.
static bool getErrorPage(ServerDirective const *servDir, const std::string &uri,
						 int code, std::string &out)
{
	ServerDirective::ResourcePath errorPath = servDir->getErrorPage(uri, code);
	if (!errorPath.first)
		return false;

	ServerDirective::ResourcePath errorPage = servDir->getResource(errorPath.second);
	std::string body;
	if (!(errorPage.first && Utils::readFile(errorPage.second, body)))
		return false;

	out = HttpResponse::build(code, MimeTypes::forPath(errorPage.second), body, "Connection: close");
	return true;
}

// Builds a response for `code` using a custom error page when configured,
// otherwise a generated default error body.
std::string HttpResponse::buildError(int code, const std::string &uri,
									 ServerDirective const *servDir)
{
	std::string response;
	if (getErrorPage(servDir, uri, code, response))
		return response;

	std::string body = HttpResponse::defaultErrorBody(code);
	return HttpResponse::build(code, "text/html", body, "Connection: close");
}

namespace
{

	struct AutoindexEntry
	{
		std::string name;
		bool isDir;
		off_t size;
		std::string mtime;
	};

#ifdef DEBUG
	const char *dirent_type_str(unsigned char t)
	{
		switch (t)
		{
		case DT_REG:
			return "file";
		case DT_DIR:
			return "dir";
		case DT_LNK:
			return "symlink";
		case DT_UNKNOWN:
			return "unknown";
		default:
			return "other";
		}
	}
#endif

	// nginx-style "%d-%b-%Y %H:%M" formatting of a file's last-modified time
	std::string formatMtime(time_t mtime)
	{
		char buf[32];
		struct tm *tmInfo = localtime(&mtime);
		strftime(buf, sizeof(buf), "%d-%b-%Y %H:%M", tmInfo);
		return std::string(buf);
	}

	std::string buildAutoindexBody(std::vector<AutoindexEntry> entries, std::string currLoc)
	{
		// top
		std::stringstream ss;
		ss << "<html><head><title>" << currLoc << "</title></head>\r\n"
		   << "<body><center><h1>Index of " << currLoc << "</h1></center>\r\n"
		   << "<table><tr><th>Name</th><th>Last Modified</th><th>Size</th></tr>\r\n";
		std::string base = currLoc;
		if (base.empty() || base[base.size() - 1] != '/')
			base += "/";
		// middle entries
		for (std::size_t i = 0; i < entries.size(); ++i)
		{
			const AutoindexEntry &e = entries[i];
			if (e.name == "." || (e.name == ".." && base == "/"))
				continue;

			std::string display = e.isDir ? e.name + "/" : e.name;

			ss << "<tr><td><a href=\"" << base << display << "\">" << display << "</a></td>"
			   << "<td>" << e.mtime << "</td>"
			   << "<td>";
			if (e.isDir)
				ss << "-";
			else
				ss << e.size;
			ss << "</td></tr>\r\n";
		}
		// bottom
		ss << "</table><hr><center>webserv</center></body></html>\r\n";

		return ss.str();
	}
}

std::string HttpResponse::buildAutoindex(const std::string &fsDir, const std::string &uriPath, const ServerDirective *servDir)
{
	DIR *d = opendir(fsDir.c_str());
	if (!d)
		return HttpResponse::buildError(403, uriPath, servDir);

	std::vector<AutoindexEntry> entries;
	struct dirent *entry;
	while ((entry = readdir(d)) != NULL)
	{
		std::string name = entry->d_name;
		std::string entryPath = fsDir + "/" + name;
		struct stat est;
		if (stat(entryPath.c_str(), &est) != 0)
			continue;

		LOG_DEBUG("entry: " << entry->d_ino << ", " << name << ", "
							<< entry->d_off << ", " << entry->d_reclen << ", "
							<< dirent_type_str(entry->d_type));

		AutoindexEntry ae;
		ae.name = name;
		ae.isDir = S_ISDIR(est.st_mode);
		ae.size = est.st_size;
		ae.mtime = formatMtime(est.st_mtime);
		entries.push_back(ae);
	}
	closedir(d);

	std::string body = buildAutoindexBody(entries, uriPath);
	return HttpResponse::build(200, "text/html", body);
}

// Generates error body if no error html resource to serve
std::string HttpResponse::defaultErrorBody(int code)
{
	const std::string &reason = HttpStat::getReason(code);

	std::stringstream ss;
	ss << "<html><head><title>" << code << " " << reason << "</title></head>\r\n"
	   << "<body><center><h1>" << code << " " << reason << "</h1></center>\r\n"
	   << "<hr><center>webserv</center></body></html>\r\n";
	return ss.str();
}

/** Turns raw CGI stdout (RFC 3875 section 6) into a full HTTP response.
 * Header lines may end in "\r\n" or "\n"; an empty line ends the headers.
 * - Status:   sets the response code and is not forwarded.
 * - Location: without a Status, the response is a 302.
 * - Content-Length / Transfer-Encoding / Connection: dropped; the server
 *   frames the body itself with Content-Length.
 * - every other header is forwarded as-is (Set-Cookie, Cache-Control, ...).
 * Returns false when the output is malformed; the caller should send a 502.
 */
bool HttpResponse::buildCgiResponse(std::string const &cgiOutput, std::string &resp)
{
	std::string contentType;
	std::string extraHeaders;
	bool hasStatus = false;
	bool hasLocation = false;
	bool hasContentType = false;
	int code = -1;

	size_t pos = 0;
	while (true)
	{
		size_t nl = cgiOutput.find('\n', pos);
		if (nl == std::string::npos)
		{
			LOG_DEBUG("CGI output has no blank line ending its headers");
			return false;
		}
		// copies the line without its '\n'
		std::string line = cgiOutput.substr(pos, nl - pos);
		// moves pos to the start of the next line
		pos = nl + 1;
		if (!line.empty() && line[line.size() - 1] == '\r')
			// removes the trailing \r
			line.erase(line.size() - 1);
		if (line.empty())
			break; // end of headers, body starts at pos

		size_t colon = line.find(':');
		if (colon == std::string::npos || colon == 0)
		{
			LOG_DEBUG("Malformed CGI header line: " << line);
			return false;
		}
		std::string name = Utils::Trim(line.substr(0, colon));
		std::string value = Utils::Trim(line.substr(colon + 1));
		std::string key = Utils::toLowercase(name);

		if (key == "status")
		{
			if (hasStatus)
			{
				LOG_DEBUG("Duplicate CGI Status header");
				return false;
			}
			code = parseCgiStatus(value);
			if (code < 100 || code > 599)
			{
				LOG_DEBUG("Invalid CGI Status header: " << value);
				return false;
			}
			hasStatus = true;
		}
		else if (key == "content-type")
		{
			if (hasContentType)
			{
				LOG_DEBUG("Duplicate CGI Content-Type header");
				return false;
			}
			hasContentType = true;
			contentType = value;
		}
		else if (key == "content-length" || key == "transfer-encoding" || key == "connection")
			continue;
		else
		{
			if (key == "location")
				hasLocation = true;
			if (!extraHeaders.empty())
				extraHeaders += CRLF;
			extraHeaders += name + ": " + value;
		}
	}

	// Content-Type is required when there is body
	if (contentType.empty() && pos < cgiOutput.size())
	{
		LOG_DEBUG("CGI response has a body but no Content-Type");
		return false; // caller sends 502
	}

	// Location header means a redirect if there is no status
	if (code == -1)
		code = hasLocation ? 302 : 200;

	resp = HttpResponse::build(code, contentType, cgiOutput.substr(pos), extraHeaders);
	return true;
}

// "Status: 404 Not Found" -> 404. Returns -1 if it does not start with a 3-digit code.
int HttpResponse::parseCgiStatus(std::string const &value)
{
	if (value.size() < 3 || (value.size() > 3 && value[3] != ' '))
		return -1;
	int code = 0;
	for (size_t i = 0; i < 3; ++i)
	{
		if (value[i] < '0' || value[i] > '9')
			return -1;
		code = code * 10 + (value[i] - '0');
	}
	return code;
}
