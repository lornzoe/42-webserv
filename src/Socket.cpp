/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/21 19:30:26 by ypua              #+#    #+#             */
/*   Updated: 2026/08/01 21:29:54 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"
#include <cstring>

Socket::Socket()
{
	// For TCP connection
	fd_ = socket(AF_INET, SOCK_STREAM, 0);
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

int Socket::bind_port(const std::string &host, const std::string &port)
{
	// Configure hints
	struct addrinfo hints = {};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	// only matters when node is NULL, it is ignored otherwise.
	hints.ai_flags = AI_PASSIVE;

	const char *node = host.empty() ? NULL : host.c_str();

	struct addrinfo *res = NULL;
	int rc = getaddrinfo(node, port.c_str(), &hints, &res);
	if (rc != 0)
	{
		std::cerr << "[webserv] getaddrinfo(" << host << ":" << port
				  << "): " << gai_strerror(rc) << std::endl;
		return -1;
	}

	// bind the first address that works
	int status = -1;
	int saved_errno = 0;
	for (struct addrinfo *p = res; p != NULL; p = p->ai_next)
	{
		if (bind(fd_, p->ai_addr, p->ai_addrlen) == 0)
		{
			status = 1;
			break;
		}
		saved_errno = errno;
	}

	freeaddrinfo(res);

	if (status == -1)
		std::cerr << "[webserv] bind(" << host << ":" << port
				  << "): " << std::strerror(saved_errno) << std::endl;

	return status;
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
	if (client_fd == -1)
		return Socket(-1);

	// Make the newly accepted client socket NON-BLOCKING as well
	// client.set_nonblocking();
	return Socket(client_fd);
}

// TODO: Rewrite this to prevent freeze on non-blocking reads
//   > 0  : Number of bytes read in this call.
//   0    : Client closed connection cleanly (FIN).
//  -1    : Error (check errno; EAGAIN/EWOULDBLOCK means "no data right now, try later").
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

		if (n > 0)
			sent += n;
		else if (n == -1)
		{
			// Return sent count for next EPOLLOUT event
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return sent;
			// Real network error (e.g. ECONNRESET, EPIPE)
			return -1;
		}
		else
			return (sent > 0) ? sent : -1;
	}

	return sent;
}

int Socket::set_nonblocking()
{
	// Retrieve existing status flags
	int flags = fcntl(fd_, F_GETFL, 0);
	if (flags == -1)
		return -1;

	return fcntl(fd_, F_SETFL, flags | O_NONBLOCK);
}

int Socket::set_reuseaddr()
{
	// Enable the option
	int opt = 1;
	// Override wait window on local port binding
	if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		return -1;

	return 1;
}

bool Socket::configure()
{
	// if (set_nonblocking() == -1)
	// 	return false;

	if (set_reuseaddr() == -1)
		return false;

	return true;
}