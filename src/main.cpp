/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/11 21:19:50 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/01 21:27:37 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <exception>

#include "Config.hpp"
#include "ServerDirective.hpp"
#include "ListenDirective.hpp"
#include "FileDescriptor.hpp"
#include "Socket.hpp"
#include <vector>

#include <sstream>
#include <sys/stat.h>
#include <sys/select.h>
#include <set>

// TODO: Parse HTTP request (method, URL, version, headers, body).
// See __references/notes.md "HTTP request parsing" for the target shape.

typedef std::pair<std::string, int> ListenAddr;
static const char* WILDCARD_HOST = "0.0.0.0";

static bool hasOverlappingBind(const std::vector<ListenAddr>& addrs)
{
	for (std::size_t i = 0; i < addrs.size(); i++)
	{
		if (addrs[i].first != WILDCARD_HOST)
			continue;

		for (std::size_t j = 0; j < addrs.size(); j++)
		{
			if (addrs[j].second == addrs[i].second
				&& addrs[j].first != WILDCARD_HOST)
			{
				std::cerr << "[webserv] Error: cannot listen on both "
						  << WILDCARD_HOST << ":" << addrs[i].second << " and "
						  << addrs[j].first << ":" << addrs[j].second
						  << " (overlapping bind)" << std::endl;
				return true;
			}
		}
	}
	return false;
}

static bool collectListens(const Config& config, std::vector<ListenAddr>& out)
{
	const std::vector<Directive *>& top = config.getDirectives();
	std::set<ListenAddr> seen;

	for (std::size_t i = 0; i < top.size(); i++)
	{
		const ServerDirective* server = dynamic_cast<const ServerDirective*>(top[i]);
		if (!server)
			continue;

		const std::vector<const ListenDirective *> listens = server->getListens();
		for (std::size_t j = 0; j < listens.size(); j++)
		{
			ListenAddr addr = std::make_pair(listens[j]->getHost(),
											 listens[j]->getPort());

			if (!seen.insert(addr).second)
			{
				std::cout << "[webserv] note: " << addr.first << ":" << addr.second
						  << " already bound by an earlier server block" << std::endl;
				continue;
			}

			out.push_back(addr);
		}
	}
	return !out.empty();
}

int runServer(const std::vector<ListenAddr>& addrs)
{
	FileDescriptor file("index.html");
	if (file.get() == -1)
		return 1;

	struct stat st;
	if (stat("index.html", &st) == -1)
		return 1;

	std::vector<char> body(st.st_size);

	ssize_t total = 0;
	while (total < st.st_size)
	{
		ssize_t bytes = read(file.get(),
							 &body[total],
							 st.st_size - total);

		if (bytes <= 0)
			return 1;

		total += bytes;
	}

	std::stringstream ss;
	ss << total;
	std::string length = ss.str();

	std::string header =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: " +
		length + HEADER_END;

	// Create server socket
	std::vector<Socket *> listeners;
	for (std::size_t i = 0; i < addrs.size(); i++)
	{
		const std::string& host = addrs[i].first;
		const int port = addrs[i].second;

		Socket* server = new Socket();
		std::stringstream port_ss;
		port_ss << port;

		if (server->get() == -1
			|| !server->configure()
			|| server->bind_port(host, port_ss.str()) == -1
			|| server->listen_connection(SOMAXCONN) == -1)
		{
			std::cerr << "[webserv] Error: failed to listen on "
					  << host << ":" << port << std::endl;
			delete server;
			for (std::size_t j = 0; j < listeners.size(); j++)
				delete listeners[j];
			return 1;
		}

		std::cout << "[webserv] listening on " << host << ":" << port << std::endl;
		listeners.push_back(server);
	}

	if (listeners.empty())
		return 1;

	std::cout << "[webserv] main loop, ctrl + c to end" << std::endl;
	while (true)
	{
		fd_set readfds;
		FD_ZERO(&readfds);
		int maxfd = -1;
		for (std::size_t i = 0; i < listeners.size(); i++)
		{
			int fd = listeners[i]->get();
			FD_SET(fd, &readfds);
			if (fd > maxfd)
				maxfd = fd;
		}

		if (select(maxfd + 1, &readfds, NULL, NULL, NULL) == -1)
		{
			for (std::size_t j = 0; j < listeners.size(); j++)
				delete listeners[j];
			return 1;
		}

		for (std::size_t i = 0; i < listeners.size(); i++)
		{
			if (!FD_ISSET(listeners[i]->get(), &readfds))
				continue;

			// accept connection
			Socket client = listeners[i]->accept_connection();
			if (client.get() == -1)
				continue;

			// Receive request
			std::string request = client.receive_all(0);
			std::cout << "Message from client: " << request << std::endl;

			// Send response
			client.send_all(header.c_str(), header.size(), 0);
			client.send_all(body.data(), body.size(), 0);
		}
	}

		for (std::size_t i = 0; i < listeners.size(); i++)
			delete listeners[i];

		// // Create epoll
		// int epoll_fd = epoll_create(1);
		// if (epoll_fd == -1)
		// 	return 1;

		// // Add server socket to epoll
		// epoll_event ev = {};
		// ev.events = EPOLLIN;
		// ev.data.fd = server.get();

		// if (epoll_ctl(epoll_fd,
		// 			  EPOLL_CTL_ADD,
		// 			  server.get(),
		// 			  &ev) == -1)
		// 	return 1;

		// // Create event storage
		// int MAX_EVENTS = 64;
		// epoll_event events[MAX_EVENTS];

		// while (true)
		// {
		// 	// Get ready file descriptors
		// 	int ready = epoll_wait(
		// 		epoll_fd,
		// 		events,
		// 		MAX_EVENTS,
		// 		-1);

		// 	if (ready == -1)
		// 		break;

		// 	// Iterate over all ready events
		// 	for (int i = 0; i < ready; i++)
		// 	{
		// 		int fd = events[i].data.fd;
		// 		if (fd == server.get())
		// 		{
		// 			// New connection
		// 			// Accept every pending client
		// 			while (true)
		// 			{
		// 				Socket client = server.accept_connection();

		// 				if (client.get() == -1)
		// 				{
		// 					if (errno == EAGAIN ||
		// 						errno == EWOULDBLOCK)
		// 					{
		// 						// No more clients waiting
		// 						break;
		// 					}

		// 					break;
		// 				}

		// 				epoll_event ev = {};
		// 				ev.events = EPOLLIN;
		// 				ev.data.fd = client.get();

		// 				epoll_ctl(epoll_fd,
		// 						  EPOLL_CTL_ADD,
		// 						  client.get(),
		// 						  &ev);

		// 				// Save the Socket object somewhere!
		// 			}
		// 		}
		// 		else
		// 		{
		// 			// Existing client
		// 			char buffer[4096];

		// 			ssize_t n = recv(fd,
		// 							 buffer,
		// 							 sizeof(buffer),
		// 							 0);

		// 			if (n == 0)
		// 			{
		// 				std::cout << "Client disconnected\n";

		// 				epoll_ctl(epoll_fd,
		// 						  EPOLL_CTL_DEL,
		// 						  fd,
		// 						  NULL);

		// 				close(fd);

		// 				continue;
		// 			}

		// 			if (n == -1)
		// 			{
		// 				if (errno == EAGAIN ||
		// 					errno == EWOULDBLOCK)
		// 					continue;

		// 				epoll_ctl(epoll_fd,
		// 						  EPOLL_CTL_DEL,
		// 						  fd,
		// 						  NULL);

		// 				close(fd);

		// 				continue;
		// 			}

		// 			std::cout.write(buffer, n);

		// 			send(fd,
		// 				 header.c_str(),
		// 				 header.size(),
		// 				 0);

		// 			send(fd,
		// 				 body.data(),
		// 				 body.size(),
		// 				 0);
		// 		}
		// 	}
		
	return 0;
}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cerr << "Usage: ./webserv <config_file>" << std::endl;
		return 1;
	}

	std::vector<ListenAddr> addrs;

	try
	{
		Config c(argv[1]);
		std::cout << "[webserv] Config() completed." << std::endl;
		// c.printConfig();
		// c.printDirectives();

		if (!collectListens(c, addrs))
		{
			std::cerr << "Warning: no 'listen' directive found in config, "
						 "defaulting to port 8080" << std::endl;
			addrs.push_back(std::make_pair(std::string(WILDCARD_HOST), 8080));
		}

		if (hasOverlappingBind(addrs))
			return 1;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	std::cout << "[webserv] running server now." << std::endl;
	return runServer(addrs);
}
