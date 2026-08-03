/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ypua <ypua@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/03 19:37:55 by ypua              #+#    #+#             */
/*   Updated: 2026/08/03 20:29:55 by ypua             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <cstddef>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

#ifndef SPACES
#define SPACES " \f\n\r\t\v"
#endif

class HttpRequest
{
public:
	HttpRequest(std::string);
	std::string get_method();
	std::string get_path();

private:
	std::string method_;
	std::string path_;
	std::string http_version_;
	// std::string query_string;

	std::map<std::string, std::string> headers;

	// Body (ignored for GET, populated for POST)
	// std::string body;

	// bool is_complete;
	// bool is_chunked;
	// size_t content_length;
};

#endif