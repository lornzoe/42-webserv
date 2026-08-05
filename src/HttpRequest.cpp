/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ypua <ypua@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/03 19:37:57 by ypua              #+#    #+#             */
/*   Updated: 2026/08/05 20:34:25 by ypua             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

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

	// Ignore any empty line(s) received where a Request-Line is expected 

	// Parse request line
	if (std::getline(stream, line))
	{
		std::istringstream iss(line);
		iss >> method_ >> path_ >> http_version_;
	}

	// Parse headers
	while (std::getline(stream, line))
	{
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
