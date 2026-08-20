/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/20 17:50:51 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/20 18:33:45 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"
#include "HttpStatus.hpp"

#include <sstream>

#define CRLF "\r\n"
#define HEADER_END "\r\n\r\n"

std::string HttpResponse::build(int code, const std::string &contentType, const std::string &body, std::string additionalHeaders)
{
	// build the header first
	std::stringstream ss;
	ss << "HTTP/1.1 " << code << " " << HttpStatus::getDefaultResponse(code) << CRLF;
	ss << "Content-Type: " << contentType << CRLF;
	if (!additionalHeaders.empty())
		ss << additionalHeaders << CRLF;
	ss << "Content-Length: " << body.size() << HEADER_END;

	// build the body
	ss << body;
	
	return ss.str();
}
