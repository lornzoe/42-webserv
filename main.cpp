/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ypua <ypua@student.42.singapore.sg>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/19 17:53:42 by ypua              #+#    #+#             */
/*   Updated: 2026/07/19 20:57:23 by ypua             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fcntl.h>
#include <unistd.h>
#include <sstream>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>

// TODO: send() part is not entirely correct
// TODO: better resource cleanup on error
// TODOl recv() is always ignored
int main(void)
{
	int fd = open("index.html", O_RDONLY);
	if (fd == -1)
		return 1;

	struct stat st;
	if (stat("index.html", &st) == -1)
		return 1;

	char *body = new char[st.st_size];
	ssize_t n = read(fd, body, st.st_size);
	if (n == -1)
		return 1;

	std::stringstream ss;
	ss << n;
	std::string length = ss.str();

	std::string header =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: " +
		length + "\r\n"
				 "\r\n";

	// Create server socket
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1)
		return 1;

	// Define server address
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(8080);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	// Bind socket to address
	if (bind(serverSocket,
			 (struct sockaddr *)&serverAddress,
			 sizeof(serverAddress)) == -1)
		return 1;

	// Listen for incoming connections
	if (listen(serverSocket, SOMAXCONN) == -1)
		return 1;

	while (true)
	{
		// Accept Client Connection
		int clientSocket = accept(serverSocket, NULL, NULL);
		if (clientSocket == -1)
			return 1;

		// Receive request
		char buffer[1024] = {0};
		ssize_t received = recv(clientSocket, buffer, sizeof(buffer), 0);
		if (received <= 0)
		{
			close(clientSocket);
			continue;
		}
		std::cout << "Message from client: " << buffer << std::endl;

		// Send response
		send(clientSocket, header.c_str(), header.size(), 0);
		send(clientSocket, body, n, 0);

		close(clientSocket);
	}

	close(fd);
	// Close Server Socket
	close(serverSocket);

	delete[] body;

	return 0;
}
