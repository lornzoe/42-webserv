/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ypua <ypua@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/03 19:37:57 by ypua              #+#    #+#             */
/*   Updated: 2026/08/27 20:23:40 by ypua             ###   ########.fr       */
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

bool isValidHttpRequest(ParsedHeaders const &req)
{
	if (req.method.empty() || req.path.empty() || req.http_version.empty())
		return false;

	if (req.http_version != "HTTP/1.1")
		return false;

	if (req.method != "GET" && req.method != "POST" && req.method != "DELETE")
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
		   ss.str() + HEADER_TERMINATOR;
}

static const std::string &contentTypeFor(const std::string &path)
{
	std::size_t dot = path.rfind('.');
	if (dot == std::string::npos)
		return MimeTypes::getContentType("");

	return MimeTypes::getContentType(path.substr(dot));
}

static bool readFile(const std::string &path, std::string &body)
{
	FileDescriptor file(path);
	if (file.get() == -1)
		return false;

	struct stat st;
	if (stat(path.c_str(), &st) == -1)
		return false;

	body.resize(st.st_size);

	ssize_t total = 0;
	while (total < st.st_size)
	{
		ssize_t bytes = read(file.get(), &body[total], st.st_size - total);
		if (bytes <= 0)
			return false;
		total += bytes;
	}
	return true;
}

std::string HttpRequest::build_http_response(ParsedHeaders const &req,
											 std::string const &body_in,
											 ServerDirective const *servDir)
{
	std::string body;
	std::string header;

	if (!isValidHttpRequest(req))
	{
		body = "<html><body><h1>400 Bad Request</h1></body></html>";
		header = buildHeader(400, "Bad Request",
							 HttpStatus::getDefaultResponse(400),
							 body.size());
		return header + body;
	}

	ServerDirective::ResourcePath resourcePath = servDir->getResource(req.path);
	bool found = resourcePath.first;
	const std::string &path = resourcePath.second;

	if (req.method == "POST")
	{
		// handle body_in
		std::cout << body_in << std::endl;
	}

	if (found && readFile(path, body))
		header = buildHeader(200, "OK", contentTypeFor(path), body.size());
	else
	{
		body = "<html><body><h1>404 Not Found</h1></body></html>";
		header = buildHeader(404, "Not Found",
							 HttpStatus::getDefaultResponse(404), body.size());
	}

	return header + body;
}

void removeTrailingCarriageReturn(std::string &line)
{
	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);
}

ParsedHeaders parseHeaderBlock(const std::string &headerBlock)
{
	ParsedHeaders result;

	std::istringstream stream(headerBlock);
	std::string line;

	// Parse the request line
	if (!std::getline(stream, line))
		return result;

	removeTrailingCarriageReturn(line);

	std::istringstream reqLine(line);
	if (!(reqLine >> result.method >> result.path >> result.http_version))
		return result;

	std::string extra;
	if (reqLine >> extra)
		return result;

	while (std::getline(stream, line))
	{
		removeTrailingCarriageReturn(line);

		if (line.empty())
			break;

		size_t pos = line.find(':');
		if (pos == std::string::npos)
			return result;

		std::string key = Utils::Trim(line.substr(0, pos));
		std::string value = Utils::Trim(line.substr(pos + 1));
		if (key.empty())
			return result;

		key = Utils::toLowercase(key);
		// Reject duplicate headers for simplification
		if (result.header_keys.count(key))
			return result;
		result.header_keys.insert(key);
		result.headers[key] = value;

		if (key == "content-length")
		{
			char *end = NULL;
			long len = std::strtol(value.c_str(), &end, 10);
			if (end == value.c_str() || *end != '\0' || len < 0)
				return result;
			result.content_length = static_cast<size_t>(len);
		}
	}

	if (result.http_version == "HTTP/1.1" && !result.header_keys.count("host"))
		return result;

	result.valid = true;
	return result;
}

ParseResult HttpRequest::parse_http_request(const std::string &inbox)
{
	static const size_t MAX_HEADER_SIZE = 8192;

	// 1. Look for end of headers
	size_t header_end = inbox.find(HEADER_TERMINATOR);
	if (header_end == std::string::npos)
	{
		if (inbox.size() > MAX_HEADER_SIZE)
			return ParseResult(INVALID, 0);
		return ParseResult(INCOMPLETE, 0);
	}

	// 2. Parse request line and headers
	std::string headers = inbox.substr(0, header_end);
	ParsedHeaders parsed = parseHeaderBlock(headers);
	if (!parsed.valid)
		return ParseResult(INVALID, 0);

	// TODO: Remove this once we support chunked
	if (parsed.headers.count("transfer-encoding"))
		return ParseResult(INVALID, 0);

	// 3. Determine body length
	size_t body_start = header_end + 4;
	size_t content_length = parsed.content_length;

	// 4. Check whether entire body has arrived
	if (inbox.size() < body_start + content_length)
		return ParseResult(INCOMPLETE, 0);

	// 5. Entire request exists
	size_t request_size = body_start + content_length;
	return ParseResult(COMPLETE, request_size, parsed);
}
