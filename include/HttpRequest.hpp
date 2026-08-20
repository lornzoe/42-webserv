/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/03 19:37:55 by ypua              #+#    #+#             */
/*   Updated: 2026/08/21 04:41:45 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <cstddef>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>

#include "FileDescriptor.hpp"
#include "HttpStatus.hpp"
#include "MimeTypes.hpp"
#include "ServerDirective.hpp"
#include "Socket.hpp"

#ifndef SPACES
#define SPACES " \f\n\r\t\v"
#endif

class HttpRequest
{
public:
	HttpRequest(std::string);
	std::string get_method();
	std::string get_path();
	std::string get_http_version();
	void show_all();
	std::string build_http_response(ServerDirective const *servDir);

private:
	std::string method_;
	std::string path_;
	std::string http_version_;
	// std::string query_string;

	std::set<std::string> header_keys;
	std::map<std::string, std::string> headers_;

	// Body (ignored for GET, populated for POST)
	std::string body_;

	// bool is_complete;
	// bool is_chunked;
	// size_t content_length;
	std::string toLowercase(std::string str);

	int requestError() const;
};

#endif
