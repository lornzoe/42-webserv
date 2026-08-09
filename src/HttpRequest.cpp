/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ypua <ypua@student.42.singapore.sg>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/03 19:37:57 by ypua              #+#    #+#             */
/*   Updated: 2026/08/09 18:03:24 by ypua             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"
#include <Socket.hpp>
#include <HttpStatus.hpp>
#include <MimeTypes.hpp>
#include <vector>
#include <ServerDirective.hpp>
#include <FileDescriptor.hpp>

#include <sys/stat.h>

// HTTP/1.1 requests containing a message-body MUST include a valid Content-Length
// header field. If a request contains a message-body and a Content-Length is
// not given,
// The server SHOULD respond with 400 (bad request) if it cannot
// determine the length of the message, or with 411 (length required) if
// it wishes to insist on receiving a valid Content-Length.

// Messages MUST NOT include both a Content-Length header field and a
// non-identity transfer-coding. If the message does include a non-
// identity transfer-coding, the Content-Length MUST be ignored.

// All responses to the HEAD request method
// MUST NOT include a message-body, even though the presence of entity-
// header fields might lead one to believe they do. All 1xx
// (informational), 204 (no content), and 304 (not modified) responses
// MUST NOT include a message-body. All other responses do include a
// message-body, although it MAY be of zero length.

// Request    = Request-Line
// 				*(( general-header
// 		  		 | request-header
// 			 	 | entity-header ) CRLF)
// 				CRLF
// 				[ message-body ]

// Example
// GET / HTTP/1.1
// Accept-Encoding: gzip, deflate, br, zstd
// Accept-Language: en-US,en;q=0.5
// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
// Connection: keep-alive
// Host: localhost:8080
// Priority: u=0, i
// Sec-Fetch-Dest: document
// Sec-Fetch-Mode: navigate
// Sec-Fetch-Site: none
// Sec-Fetch-User: ?1
// Upgrade-Insecure-Requests: 1
// User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:142.0) Gecko/20100101 Firefox/142.0

// An origin server SHOULD return the status code 405 (Method Not Allowed)
// if the method is known by the origin server but not allowed for the
// requested resource, and 501 (Not Implemented) if the method is
// unrecognized or not implemented by the origin server.

// 1. If Request-URI is an absoluteURI, the host is part of the
//    Request-URI. Any Host header field value in the request MUST be
//    ignored.

// 2. If the Request-URI is not an absoluteURI, and the request includes
//    a Host header field, the host is determined by the Host header
//    field value.

// 3. If the host as determined by rule 1 or 2 is not a valid host on
//    the server, the response MUST be a 400 (Bad Request) error message.
void LeftTrim(std::string &s)
{
	size_t found = s.find_first_not_of(SPACES);
	if (found != std::string::npos)
		s.erase(0, found);
	else
		s.clear();
}

void RightTrim(std::string &s)
{
	size_t found = s.find_last_not_of(SPACES);
	if (found != std::string::npos)
		s.erase(found + 1);
	else
		s.clear();
}

std::string Trim(std::string s)
{
	LeftTrim(s);
	RightTrim(s);
	return s;
}

HttpRequest::HttpRequest(std::string request)
{
	std::istringstream stream(request);
	std::string line;

	// Parse request line
	while (std::getline(stream, line))
	{
		// Ignore any empty line(s) received
		if (Trim(line).empty())
			continue;

		std::istringstream iss(line);
		iss >> method_ >> path_ >> http_version_;
		break;
	}

	// Parse headers
	while (std::getline(stream, line))
	{
		// Empty line means end of headers
		if (Trim(line).empty())
			break;

		size_t pos = line.find(':');
		if (pos != std::string::npos)
		{
			std::string key = Trim(line.substr(0, pos));
			std::string value = Trim(line.substr(pos + 1));

			key = toLowercase(key);
			header_keys.insert(key);
			headers_[key] = value;
		}
	}

	// Read bodys
	// TODO: should use Content-Length to determine how many bytes belong to the body
	body_ = std::string(std::istreambuf_iterator<char>(stream),
						std::istreambuf_iterator<char>());
}

std::string HttpRequest::get_method()
{
	if (method_.empty())
		throw std::runtime_error("HTTP method not set");

	return method_;
}

std::string HttpRequest::get_path()
{
	if (path_.empty())
		throw std::runtime_error("HTTP path not set");

	return path_;
}

std::string HttpRequest::get_http_version()
{
	if (http_version_.empty())
		throw std::runtime_error("HTTP version not set");

	return http_version_;
}

void HttpRequest::show_all()
{
	std::cout << get_method() << " "
			  << get_path() << " "
			  << get_http_version() << std::endl;

	for (std::set<std::string>::iterator itr = header_keys.begin();
		 itr != header_keys.end();
		 ++itr)
		std::cout << *itr << ": " << headers_.at(*itr) << std::endl;

	std::cout << body_ << std::endl;
}

std::string HttpRequest::toLowercase(std::string str)
{
	for (size_t j = 0; j < str.length(); j++)
		str[j] = static_cast<char>(tolower(static_cast<unsigned char>(str[j])));
	return str;
}

bool HttpRequest::isValidHttpRequest()
{
	if (method_.empty() || path_.empty() || http_version_.empty())
		return false;

	if (http_version_ != "HTTP/1.1")
		return false;

	if (method_ != "GET" &&
		method_ != "POST" && method_ != "DELETE")
		return false;

	return true;
}

static std::string buildHeader(int code, const std::string &reason,
							   const std::string &contentType, std::size_t length)
{
	std::stringstream ss;
	ss << length;

	std::stringstream statusLine;
	statusLine << "HTTP/1.1 " << code << " " << reason << "\r\n";

	return statusLine.str() +
		   "Content-Type: " + contentType + "\r\n" +
		   "Content-Length: " +
		   ss.str() + HEADER_END;
}

// static const std::string &contentTypeFor(const std::string &path)
// {
// 	std::size_t dot = path.rfind('.');
// 	if (dot == std::string::npos)
// 		return MimeTypes::getContentType("");

// 	return MimeTypes::getContentType(path.substr(dot));
// }

// static bool readFile(const std::string &path, std::vector<char> &body)
// {
// 	FileDescriptor file(path);
// 	if (file.get() == -1)
// 		return false;

// 	struct stat st;
// 	if (stat(path.c_str(), &st) == -1)
// 		return false;

// 	body.resize(st.st_size);

// 	ssize_t total = 0;
// 	while (total < st.st_size)
// 	{
// 		ssize_t bytes = read(file.get(), &body[total], st.st_size - total);
// 		if (bytes <= 0)
// 			return false;
// 		total += bytes;
// 	}
// 	return true;
// }

std::string HttpRequest::build_http_response()
{
	if (!isValidHttpRequest())
	{
		std::cout << "[webserv] return 400" << std::endl;
		static const std::string badRequestBody = "<html><body><h1>400 Bad Request</h1></body></html>";
		return buildHeader(400, "Bad Request", "text/html", badRequestBody.size()) + badRequestBody;
	}

	std::cout << "[webserv] return 200" << std::endl;
	static const std::string body = "<html><body><h1>200 OK</h1></body></html>";
	return buildHeader(200, "OK", "text/html", body.size()) + body;

	// ServerDirective::ResourcePath resourcePath = servers[i]->getResource(path_);
	// bool found = resourcePath.first;
	// const std::string &path = resourcePath.second;

	// 	std::vector<char> body;
	// 	if (found && readFile(path, body))
	// 		return buildHeader(200, "OK", contentTypeFor(path), body.size());
	// 	else
	// 	{
	// 		static const std::string notFoundBody = "<html><body><h1>404 Not Found</h1></body></html>";
	// 		return buildHeader(404, "Not Found", HttpStatus::getDefaultResponse(404), notFoundBody.size());
	// 	}
	// }
}
