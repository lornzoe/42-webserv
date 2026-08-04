/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ypua <ypua@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/03 19:37:55 by ypua              #+#    #+#             */
/*   Updated: 2026/08/04 20:07:00 by ypua             ###   ########.fr       */
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
};

#endif