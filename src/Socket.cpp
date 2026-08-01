/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ypua <ypua@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/21 19:30:26 by ypua              #+#    #+#             */
/*   Updated: 2026/07/22 19:13:29 by ypua             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

Socket::Socket()
{
	fd_ = socket(AF_INET, SOCK_STREAM, 0);
	if (fd_ != -1)
	{
		int opt = 1;
		setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	}
}

Socket::Socket(int fd) : fd_(fd)
{
}

Socket::~Socket()
{
	if (fd_ != -1)
		close(fd_);
}

int Socket::get()
{
	return fd_;
}

int Socket::bind_port(unsigned long port)
{
	// Define server address
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	if (bind(fd_,
			 (struct sockaddr *)&serverAddress,
			 sizeof(serverAddress)) == -1)
		return -1;

	return 1;
}

int Socket::listen_connection(int N)
{
	if (listen(fd_, N) == -1)
		return -1;

	return 1;
}

Socket Socket::accept_connection()
{
	int client_fd = accept(fd_, NULL, NULL);
	return Socket(client_fd);
}

std::string Socket::receive_all(int flag)
{
	std::string request;
	char buffer[4096];

	while (true)
	{
		ssize_t n = recv(fd_, buffer, sizeof(buffer), flag);
		 if (n <= 0)
            break;

		request.append(buffer, n);

		if (request.find(HEADER_END) != std::string::npos)
            break;
	}

	return request;
}

ssize_t Socket::send_all(const char *buffer, size_t len, int flag)
{
	size_t sent = 0;

	while (sent < len)
	{
		ssize_t n = send(fd_, buffer + sent, len - sent, flag);

		if (n <= 0)
			return -1;

		sent += n;
	}

	return sent;
}