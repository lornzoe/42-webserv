/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ypua <ypua@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/03 19:37:57 by ypua              #+#    #+#             */
/*   Updated: 2026/08/03 20:30:04 by ypua             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"


// void LeftTrim(std::string &s)
// {
// 	size_t found = s.find_first_not_of(SPACES);
// 	if (found != std::string::npos)
// 		s.erase(0, found);
// 	else
// 		s.clear();
// }

// void RightTrim(std::string &s)
// {
// 	size_t found = s.find_last_not_of(SPACES);
// 	if (found != std::string::npos)
// 		s.erase(found + 1);
// 	else
// 		s.clear();
// }

// void Trim(std::string &s)
// {
// 	LeftTrim(s);
// 	RightTrim(s);
// }

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
	// while (std::getline(stream, line))
	// 	std::cout << line << std::endl;

	// Read body
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