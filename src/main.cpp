/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/11 21:19:50 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/01 19:59:11 by lyanga           ###   ########.fr       */
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

#include <sys/epoll.h>

// TODO: Parse HTTP request into Method, Request URL, HTTP Version + Host,
// Headers: Content-Length, Transfer-Encoding and Content-Type, Body

// Example:
// GET /favicon.ico HTTP/1.1
// Host: localhost:8080
// User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:142.0) Gecko/20100101 Firefox/142.0
// Accept: image/avif,image/webp,image/png,image/svg+xml,image/*;q=0.8,*/*;q=0.5
// Accept-Language: en-US,en;q=0.5
// Accept-Encoding: gzip, deflate, br, zstd
// Connection: keep-alive
// Referer: http://localhost:8080/
// Sec-Fetch-Dest: image
// Sec-Fetch-Mode: no-cors
// Sec-Fetch-Site: same-origin
// Priority: u=6

// Finds the first "listen" directive under the first "server" block in the
// parsed config. Returns false if none is found (caller should fall back).
static bool findFirstListen(const Config& config, std::string& host, int& port)
{
	const std::vector<Directive *>& top = config.getDirectives();
	for (std::size_t i = 0; i < top.size(); i++)
	{
		const ServerDirective* server = dynamic_cast<const ServerDirective*>(top[i]);
		if (!server)
			continue;

		const std::vector<const ListenDirective *> listens = server->getListens();
		if (!listens.empty())
		{
			host = listens[0]->getHost();
			port = listens[0]->getPort();
			std::cout << "[webserv] connecting to host "
					<< host << " on port " << port << std::endl;
			return true;
		}
	}
	return false;
}

int runServer(const std::string& host, int port)
{
	(void)host; // Socket::bind_port only supports INADDR_ANY for now

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
	Socket server;
	if (server.get() == -1)
		return 1;

	// Configure socket to be reuseable and non-blocking
	if (!server.configure())
		return 1;

	std::cout << "[webserv] binding port" << std::endl;
	std::stringstream port_ss;
	port_ss << port;

	// Bind socket to address
	if (server.bind_port(port_ss.str()) == -1)
		return 1;

	// Listen for incoming connections
	if (server.listen_connection(SOMAXCONN) == -1)
		return 1;

	std::cout << "[webserv] main loop, ctrl + c to end" << std::endl;
	while (true)
	{
		// Accept Client Connection
		Socket client = server.accept_connection();
		if (client.get() == -1)
			return 1;

		// Receive request
		std::string request = client.receive_all(0);
		std::cout << "Message from client: " << request << std::endl;

		// Send response
		client.send_all(header.c_str(), header.size(), 0);
		client.send_all(body.data(), body.size(), 0);

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
	}

	return 0;
}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cerr << "Usage: ./webserv <config_file>" << std::endl;
		return 1;
	}

	std::string host;
	int port = 8080;

	try
	{
		Config c(argv[1]);
		std::cout << "[webserv] Config() completed." << std::endl;
		// c.printConfig();
		// c.printDirectives();

		if (!findFirstListen(c, host, port))
		{
			std::cerr << "Warning: no 'listen' directive found in config, "
						 "defaulting to port 8080" << std::endl;
			host = "";
			port = 8080;
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	std::cout << "[webserv] running server now." << std::endl;
	return runServer(host, port);
}
