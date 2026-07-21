/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ypua <ypua@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/19 17:53:42 by ypua              #+#    #+#             */
/*   Updated: 2026/07/21 20:09:19 by ypua             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FileDescriptor.hpp"
#include "Socket.hpp"
#include <vector>

#include <sstream>
#include <sys/stat.h>
#include <iostream>

// TODO: properly parse HTTP request instead of ignoring received data
int main(void)
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
		length + "\r\n"
				 "\r\n";

	// Create server socket
	Socket server;
	if (server.get() == -1)
		return 1;

	// Bind socket to address
	if (server.bind_port(8080) == -1)
		return 1;

	// Listen for incoming connections
	if (server.listen_connection(SOMAXCONN) == -1)
		return 1;

	while (true)
	{
		// Accept Client Connection
		Socket client = server.accept_connection();
		if (client.get() == -1)
			return 1;

		// Receive request
		char buffer[1024] = {0};
		ssize_t received = client.receive(buffer, sizeof(buffer), 0);
		if (received <= 0)
			continue;
		std::cout << "Message from client: " << buffer << std::endl;

		// Send response
		client.send_all(header.c_str(), header.size(), 0);
		client.send_all(body.data(), body.size(), 0);
	}

	return 0;
}
