/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: julhong <julhong@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/03 19:37:55 by ypua              #+#    #+#             */
/*   Updated: 2026/09/03 16:11:29 by julhong          ###   ########.fr       */
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
	std::string path;
	std::string query;
	std::string http_version;
	std::map<std::string, std::string> headers;
	std::set<std::string> header_keys;
	size_t content_length;

	std::string body;

	ParsedRequest() : valid(false), content_length(0) {}
};

struct ParseResult
{
	ParseStatus 	status;
	int				errorCode;	//http error status code in case of INVALID
	size_t			consumed;
	ParsedRequest	request;

	ParseResult(ParseStatus s, int ec = 400, size_t c = 0, ParsedRequest h = ParsedRequest())
		: status(s), errorCode(ec), consumed(c), request(h) {}
};

class HttpRequest
{
public:
	static ParseResult parse_http_request(const std::string &inbox);
};

#endif
