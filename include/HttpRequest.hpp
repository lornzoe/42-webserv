/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ypua <ypua@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/03 19:37:55 by ypua              #+#    #+#             */
/*   Updated: 2026/09/01 20:25:49 by ypua             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <map>
#include <set>
#include <sstream>
#include <sys/stat.h>
#include <stdlib.h>

#include "FileDescriptor.hpp"
#include "HttpStatus.hpp"
#include "MimeTypes.hpp"
#include "ServerDirective.hpp"

#include "Utils.hpp"

static const std::string HEADER_TERMINATOR = "\r\n\r\n";
static const std::string CRLF = "\r\n";

enum ParseStatus
{
	INCOMPLETE,
	COMPLETE,
	INVALID,
};

struct ParsedRequest
{
	bool valid;
	
	std::string method;
	std::string path;
	std::string http_version;
	std::map<std::string, std::string> headers;
	std::set<std::string> header_keys;
	size_t content_length;

	std::string body;

	ParsedRequest() : valid(false), content_length(0) {}
};

struct ParseResult
{
	ParseStatus status;
	size_t consumed;
	ParsedRequest request;

	ParseResult(ParseStatus s, size_t c, ParsedRequest h = ParsedRequest())
		: status(s), consumed(c), request(h)
	{
	}
};

class HttpRequest
{
public:
	static std::string build_http_response(ParsedRequest const &req,
										   std::string const &body,
										   ServerDirective const *servDir);
	static ParseResult parse_http_request(const std::string &inbox);
};

#endif
