/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ypua <ypua@student.42.singapore.sg>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/03 19:37:55 by ypua              #+#    #+#             */
/*   Updated: 2026/08/22 16:39:49 by ypua             ###   ########.fr       */
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

enum ParseStatus
{
	INCOMPLETE,
	COMPLETE,
	INVALID,
};

struct ParsedHeaders
{
	bool valid;
	std::string method;
	std::string path;
	std::string http_version;
	std::map<std::string, std::string> headers;
	std::set<std::string> header_keys;
	size_t content_length;

	ParsedHeaders() : valid(false), content_length(0) {}
};

struct ParseResult
{
	ParseStatus status;
	size_t consumed;
	ParsedHeaders header;

	ParseResult(ParseStatus s, size_t c, ParsedHeaders h = ParsedHeaders())
		: status(s), consumed(c), header(h)
	{
	}
};

class HttpRequest
{
public:
	static std::string build_http_response(ParsedHeaders const &req,
										   std::string const &body,
										   ServerDirective const *servDir);
	static ParseResult parse_http_request(const std::string &inbox);
};

#endif
