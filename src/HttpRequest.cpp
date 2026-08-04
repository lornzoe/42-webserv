/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ypua <ypua@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/03 19:37:57 by ypua              #+#    #+#             */
/*   Updated: 2026/08/04 20:09:30 by ypua             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

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