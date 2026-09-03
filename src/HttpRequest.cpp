/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ypua <ypua@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/03 19:37:57 by ypua              #+#    #+#             */
/*   Updated: 2026/09/02 19:33:44 by ypua             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

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

// Returns 0 when the request is valid, otherwise the HTTP status code that
// should be sent back to the client.
int checkValidHttpRequest(ParsedRequest const &req)
{
	if (req.method.empty() || req.path.empty() || req.http_version.empty())
		return 400;

	if (req.http_version.size() < 5 || req.http_version.compare(0, 5, "HTTP/") != 0)
		return 400;

	if (req.http_version != "HTTP/1.1")
		return 505;

	if (req.method != "GET" && req.method != "POST" && req.method != "DELETE")
		return 501;

	bool hasContentLength = req.header_keys.count("content-length");
	bool hasTransferEncoding = req.header_keys.count("transfer-encoding");
	if (!hasTransferEncoding)
	{
		if (!req.body.empty() && !hasContentLength)
			return 400;
	}

	return 0;
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
	if (!(errorPage.first && readFile(errorPage.second, body)))
		return false;

	out = HttpResponse::build(code, contentTypeFor(errorPage.second), body, "Connection: close");
	return true;
}

// Builds a response for `code` using a custom error page when configured,
// otherwise a generated default error body.
static std::string buildErrorResponse(ServerDirective const *servDir,
									  const std::string &uri, int code)
{
	std::string response;
	if (getErrorPage(servDir, uri, code, response))
		return response;

	std::string body = HttpResponse::defaultErrorBody(code);
	return HttpResponse::build(code, "text/html", body, "Connection: close");
}

std::string HttpRequest::build_http_response(ParsedRequest const &req,
											 std::string const &body_in,
											 ServerDirective const *servDir)
{
	std::string body;

	int errorCode = checkValidHttpRequest(req);
	if (errorCode)
		return buildErrorResponse(servDir, req.path, errorCode);

	// TODO: Should only run when it is looking for html?
	ServerDirective::ResourcePath resourcePath = servDir->getResource(req.path);
	bool found = resourcePath.first;
	const std::string &path = resourcePath.second;

	if (req.method == "POST")
	{
		// handle body_in
		std::cout << "Body: " << body_in << std::endl;
	}

	// TODO: Should only run for GET
	if (!(found && readFile(path, body)))
		return buildErrorResponse(servDir, req.path, 404);

	return HttpResponse::build(200, contentTypeFor(path), body);
}

void removeTrailingCarriageReturn(std::string &line)
{
	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);
}

ParsedRequest parseHeaderBlock(const std::string &headerBlock)
{
	ParsedRequest result;

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

ParseResult parseChunkedRequest(const std::string &inbox,
								size_t header_length,
								ParsedRequest &parsed)
{
	size_t pos = header_length;
	std::string body;

	while (true)
	{
		// Find the end of the chunk-size line
		size_t line_end = inbox.find(CRLF, pos);

		if (line_end == std::string::npos)
			return ParseResult(INCOMPLETE, 0);

		// Extract chunk size
		std::string size_str = inbox.substr(pos, line_end - pos);

		// Convert hexadecimal size
		char *end = NULL;
		unsigned long chunk_size =
			std::strtoul(size_str.c_str(), &end, 16);

		if (end == size_str.c_str() || *end != '\0')
			return ParseResult(INVALID, 0);

		// Move past "size\r\n"
		pos = line_end + 2;

		// Last chunk
		if (chunk_size == 0)
		{
			// Need final CRLF
			if (inbox.size() < pos + 2)
				return ParseResult(INCOMPLETE, 0);

			if (inbox.substr(pos, 2) != CRLF)
				return ParseResult(INVALID, 0);

			pos += 2;

			parsed.body = body;

			return ParseResult(COMPLETE, pos, parsed);
		}

		// Check if entire chunk presents
		if (inbox.size() < pos + chunk_size + 2)
			return ParseResult(INCOMPLETE, 0);

		// Extract chunk data
		body.append(inbox, pos, chunk_size);

		pos += chunk_size;

		// Chunk data must be followed by CRLF
		if (inbox.substr(pos, 2) != CRLF)
			return ParseResult(INVALID, 0);

		pos += 2;
	}
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
	ParsedRequest parsed = parseHeaderBlock(headers);
	if (!parsed.valid)
		return ParseResult(INVALID, 0);

	size_t header_length = header_end + 4;

	// TODO: Build body for Multipart/form-data
	std::map<std::string, std::string>::const_iterator it = parsed.headers.find("transfer-encoding");
	if (it != parsed.headers.end())
	{
		std::string value = Utils::toLowercase(it->second);
		if (value == "chunked")
		{
			return parseChunkedRequest(inbox, header_length, parsed);
		}

		return ParseResult(INVALID, 0);
	}

	// 3. Determine body length
	size_t content_length = parsed.content_length;

	// 4. Check whether entire body has arrived
	if (inbox.size() < header_length + content_length)
		return ParseResult(INCOMPLETE, 0);

	parsed.body = inbox.substr(header_length, content_length);

	// 5. Entire request exists
	size_t request_size = header_length + content_length;
	return ParseResult(COMPLETE, request_size, parsed);
}