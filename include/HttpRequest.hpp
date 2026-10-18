/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ypua <ypua@student.42.singapore.sg>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/03 19:37:55 by ypua              #+#    #+#             */
/*   Updated: 2026/10/18 18:40:40 by ypua             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <map>
#include <set>
#include <sstream>
#include <sys/stat.h>
#include <stdlib.h>

#include "w_logger.hpp"
#include "FileDescriptor.hpp"
#include "HttpStat.hpp"
#include "Utils.hpp"

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
	std::string raw_path;
	std::string path;
	std::string query;
	std::string http_version;
	std::string host;
	std::map<std::string, std::string> headers;
	std::set<std::string> header_keys;
	size_t content_length;

	std::string body;

	ParsedRequest() : valid(false), content_length(0) {}

	std::string getHeader(const std::string &key) const;
};

struct ParseResult
{
	ParseStatus status;
	int errorCode; // http error status code in case of INVALID
	size_t consumed;
	ParsedRequest request;

	ParseResult() {};
	ParseResult(ParseStatus s, int ec = 400, size_t c = 0, ParsedRequest h = ParsedRequest())
		: status(s), errorCode(ec), consumed(c), request(h) {}
};

class HttpRequest
{
public:
	static ParseResult parse_http_request(const std::string &inbox);
};

#endif
