/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/20 17:50:51 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/21 04:26:29 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"
#include "HttpStatus.hpp"

#include <sstream>

#define CRLF "\r\n"
#define HEADER_END "\r\n\r\n"

std::string HttpResponse::build(int code, const std::string &contentType, const std::string &body, const std::string additionalHeaders)
{
	// 1xx and 204 has no body and no Content-Length;
	// 304 has no body
	bool noBody = (code >= 100 && code < 200) || code == 204 || code == 304;
	bool noLength = (code >= 100 && code < 200) || code == 204;

	// build the header first
	std::stringstream ss;
	ss << "HTTP/1.1 " << code << " " << HttpStatus::getDefaultResponse(code) << CRLF;
	if (!contentType.empty())
		ss << "Content-Type: " << contentType << CRLF;
	if (!additionalHeaders.empty())
		ss << additionalHeaders << CRLF;
	if (!noLength)
		ss << "Content-Length: " << (noBody ? 0 : body.size()) << HEADER_END;
	else
		ss << CRLF; // second CRLF to signify header_end

	// build the body
	if (!noBody)
		ss << body;
	
	return ss.str();
}

std::string HttpResponse::defaultErrorBody(int code)
{
	const std::string &reason = HttpStatus::getDefaultResponse(code);

	std::stringstream ss;
	ss << "<html><head><title>" << code << " " << reason << "</title></head>\r\n"
		<< "<body><center><h1>" << code << " " << reason << "</h1></center>\r\n"
		<< "<hr><center>webserv</center></body></html>\r\n";
	return ss.str();
}