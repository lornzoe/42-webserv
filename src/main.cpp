/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ypua <ypua@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/19 17:53:42 by ypua              #+#    #+#             */
/*   Updated: 2026/07/22 19:27:51 by ypua             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FileDescriptor.hpp"
#include "Socket.hpp"
#include <vector>

#include <sstream>
#include <sys/stat.h>

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
		length + HEADER_END;

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
		std::string request = client.receive_all(0);
		std::cout << "Message from client: " << request << std::endl;

		// Send response
		client.send_all(header.c_str(), header.size(), 0);
		client.send_all(body.data(), body.size(), 0);
	}

	return 0;
}
