/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ypua <ypua@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/11 21:19:50 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/12 18:59:57 by ypua             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <exception>

#include "Config.hpp"
#include "ServerDirective.hpp"
#include "ListenDirective.hpp"
#include "FileDescriptor.hpp"
#include "Socket.hpp"
#include "HttpStatus.hpp"
#include "MimeTypes.hpp"
#include <vector>

#include "HttpRequest.hpp"

#include <sstream>
#include <sys/select.h>
#include <set>

#include "WSApp.hpp"

// typedef std::pair<std::string, int> ListenAddr;
// static const char* WILDCARD_HOST = "0.0.0.0";

// static bool hasOverlappingBind(const std::vector<ListenAddr>& addrs)
// {
// 	for (std::size_t i = 0; i < addrs.size(); i++)
// 	{
// 		if (addrs[i].first != WILDCARD_HOST)
// 			continue;

// 		for (std::size_t j = 0; j < addrs.size(); j++)
// 		{
// 			if (addrs[j].second == addrs[i].second
// 				&& addrs[j].first != WILDCARD_HOST)
// 			{
// 				std::cerr << "[webserv] Error: cannot listen on both "
// 						  << WILDCARD_HOST << ":" << addrs[i].second << " and "
// 						  << addrs[j].first << ":" << addrs[j].second
// 						  << " (overlapping bind)" << std::endl;
// 				return true;
// 			}
// 		}
// 	}
// 	return false;
// }

// static bool collectListens(const Config& config, std::vector<ListenAddr>& out,
// 						   std::vector<const ServerDirective *>& outServers)
// {
// 	const std::vector<Directive *>& top = config.getDirectives();
// 	std::set<ListenAddr> seen;

// 	for (std::size_t i = 0; i < top.size(); i++)
// 	{
// 		const ServerDirective* server = dynamic_cast<const ServerDirective*>(top[i]);
// 		if (!server)
// 			continue;

// 		const std::vector<const ListenDirective *> listens = server->getListens();
// 		for (std::size_t j = 0; j < listens.size(); j++)
// 		{
// 			ListenAddr addr = std::make_pair(listens[j]->getHost(),
// 											 listens[j]->getPort());

// 			if (!seen.insert(addr).second)
// 			{
// 				std::cout << "[webserv] note: " << addr.first << ":" << addr.second
// 						  << " already bound by an earlier server block" << std::endl;
// 				continue;
// 			}

// 			out.push_back(addr);
// 			outServers.push_back(server);
// 		}
// 	}
// 	return !out.empty();
// }

// // Minimal stand-in for the real HTTP request parser (owned separately):
// // pulls the request-target out of the request line "METHOD <URI> VERSION".
// static std::string extractRequestUri(const std::string& request)
// {
// 	std::size_t lineEnd = request.find("\r\n");
// 	std::string line = (lineEnd == std::string::npos) ? request : request.substr(0, lineEnd);

// 	std::size_t methodEnd = line.find(' ');
// 	if (methodEnd == std::string::npos)
// 		return "/";

// 	std::size_t uriEnd = line.find(' ', methodEnd + 1);
// 	if (uriEnd == std::string::npos)
// 		return "/";

// 	return line.substr(methodEnd + 1, uriEnd - methodEnd - 1);
// }

// static const std::string& contentTypeFor(const std::string& path)
// {
// 	std::size_t dot = path.rfind('.');
// 	if (dot == std::string::npos)
// 		return MimeTypes::getContentType("");

// 	return MimeTypes::getContentType(path.substr(dot));
// }

// static bool readFile(const std::string& path, std::vector<char>& body)
// {
// 	FileDescriptor file(path);
// 	if (file.get() == -1)
// 		return false;

// 	struct stat st;
// 	if (stat(path.c_str(), &st) == -1)
// 		return false;

// 	body.resize(st.st_size);

// 	ssize_t total = 0;
// 	while (total < st.st_size)
// 	{
// 		ssize_t bytes = read(file.get(), &body[total], st.st_size - total);
// 		if (bytes <= 0)
// 			return false;
// 		total += bytes;
// 	}
// 	return true;
// }

// static std::string buildHeader(int code, const std::string& reason,
// 							   const std::string& contentType, std::size_t length)
// {
// 	std::stringstream ss;
// 	ss << length;

// 	std::stringstream statusLine;
// 	statusLine << "HTTP/1.1 " << code << " " << reason << "\r\n";

// 	return statusLine.str() +
// 		"Content-Type: " + contentType + "\r\n"
// 		"Content-Length: " + ss.str() + HEADER_END;
// }

// int runServer(const std::vector<ListenAddr>& addrs,
// 			 const std::vector<const ServerDirective *>& servers)
// {
// 	// Create server socket
// 	std::vector<Socket *> listeners;
// 	for (std::size_t i = 0; i < addrs.size(); i++)
// 	{
// 		const std::string& host = addrs[i].first;
// 		const int port = addrs[i].second;

// 		Socket* server = new Socket();
// 		std::stringstream port_ss;
// 		port_ss << port;

// 		if (server->get() == -1
// 			|| !server->configure()
// 			|| server->bind_port(host, port_ss.str()) == -1
// 			|| server->listen_connection(SOMAXCONN) == -1)
// 		{
// 			std::cerr << "[webserv] Error: failed to listen on "
// 					  << host << ":" << port << std::endl;
// 			delete server;
// 			for (std::size_t j = 0; j < listeners.size(); j++)
// 				delete listeners[j];
// 			return 1;
// 		}

// 		std::cout << "[webserv] listening on " << host << ":" << port << std::endl;
// 		listeners.push_back(server);
// 	}

// 	if (listeners.empty())
// 		return 1;

// 	std::cout << "[webserv] main loop, ctrl + c to end" << std::endl;
// 	while (true)
// 	{
// 		fd_set readfds;
// 		FD_ZERO(&readfds);
// 		int maxfd = -1;
// 		for (std::size_t i = 0; i < listeners.size(); i++)
// 		{
// 			int fd = listeners[i]->get();
// 			FD_SET(fd, &readfds);
// 			if (fd > maxfd)
// 				maxfd = fd;
// 		}

// 		if (select(maxfd + 1, &readfds, NULL, NULL, NULL) == -1)
// 		{
// 			for (std::size_t j = 0; j < listeners.size(); j++)
// 				delete listeners[j];
// 			return 1;
// 		}

// 		for (std::size_t i = 0; i < listeners.size(); i++)
// 		{
// 			if (!FD_ISSET(listeners[i]->get(), &readfds))
// 				continue;

// 			// accept connection
// 			Socket client = listeners[i]->accept_connection();
// 			if (client.get() == -1)
// 				continue;

// 			// Receive request
// 			std::string request = client.receive_all(0);

// 			// std::cout << "Message from client: " << request << std::endl;

// 			std::string uri = extractRequestUri(request);

// 			ServerDirective::ResourcePath resourcePath = servers[i]->getResource(uri);
// 			bool found = resourcePath.first;
// 			const std::string& path = resourcePath.second;

// 			std::vector<char> body;
// 			std::string header;

// 			if (found && readFile(path, body))
// 			{
// 				std::cout << "[webserv] return 200 for uri: " << uri << ", path: " << path << std::endl;
// 				header = buildHeader(200, "OK", contentTypeFor(path), body.size());
// 			}
// 			else
// 			{
// 				std::cout << "[webserv] return 404 for uri: " << uri << std::endl;

// 				static const std::string notFoundBody = "<html><body><h1>404 Not Found</h1></body></html>";
// 				body.assign(notFoundBody.begin(), notFoundBody.end());
// 				header = buildHeader(404, HttpStatus::getDefaultResponse(404), "text/html", body.size());
// 			}

// 			// Send response
// 			client.send_all(header.c_str(), header.size(), 0);
// 			if (!body.empty())
// 				client.send_all(&body[0], body.size(), 0);
// 		}
// 	}

// 		for (std::size_t i = 0; i < listeners.size(); i++)
// 			delete listeners[i];
// 	return 0;
// }

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cerr << "Usage: ./webserv <config_file>" << std::endl;
		return 1;
	}
	WSApp	ws;
	try
	{
		Config::init(argv[1]);
		std::cout << "[webserv] Config() completed." << std::endl;
		ws.ConfigInit(Config::getInstance());
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	std::cout << "[webserv] running server now." << std::endl;
	ws.run();
	return 0;
}
