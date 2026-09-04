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
#include "HttpStat.hpp"
#include "ServerDirective.hpp"
#include "MimeTypes.hpp"
#include "Utils.hpp"

#include <sstream>

/** Full NON-ERROR http response, provided the relevant parts
 * contentType -- use getContentType(resource_path) to pass appropriate MIME type
 * additionalHeaders -- formatted in bulk, separated by CRLF if >1
 * body -- usually the resource (html, image, text file etc), unless to be generated e.g. defaultErrorBody
 */
std::string HttpResponse::build(int code, const std::string &contentType, const std::string &body, const std::string additionalHeaders)
{
	// 1xx and 204 has no body and no Content-Length;
	// 304 has no body
	bool noBody = (code >= 100 && code < 200) || code == 204 || code == 304;
	bool noLength = (code >= 100 && code < 200) || code == 204;

	// build the header first
	std::stringstream ss;
	ss << "HTTP/1.1 " << code << " " << HttpStat::getReason(code) << CRLF;
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

// Looks up a configured custom error page for the given uri/code and, when one
// exists and can be read, fills `out` with a ready-to-send response.
static bool		getErrorPage(ServerDirective const *servDir, const std::string &uri,
						 int code, std::string &out)
{
	ServerDirective::ResourcePath errorPath = servDir->getErrorPage(uri, code);
	if (!errorPath.first)
		return false;

	ServerDirective::ResourcePath errorPage = servDir->getResource(errorPath.second);
	std::string body;
	if (!(errorPage.first && Utils::readFile(errorPage.second, body)))
		return false;

	out = HttpResponse::build(code, MimeTypes::forPath(errorPage.second), body, "Connection: close");
	return true;
}

// Builds a response for `code` using a custom error page when configured,
// otherwise a generated default error body.
std::string		HttpResponse::buildError(int code, const std::string &uri,
											ServerDirective const *servDir)
{
	std::string response;
	if (getErrorPage(servDir, uri, code, response))
		return response;

	std::string body = HttpResponse::defaultErrorBody(code);
	return HttpResponse::build(code, "text/html", body, "Connection: close");
}

// Generates error body if no error html resource to serve
std::string HttpResponse::defaultErrorBody(int code)
{
	const std::string &reason = HttpStat::getReason(code);

	std::stringstream ss;
	ss << "<html><head><title>" << code << " " << reason << "</title></head>\r\n"
		<< "<body><center><h1>" << code << " " << reason << "</h1></center>\r\n"
		<< "<hr><center>webserv</center></body></html>\r\n";
	return ss.str();
}