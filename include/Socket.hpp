/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/21 19:30:23 by ypua              #+#    #+#             */
/*   Updated: 2026/08/01 21:07:49 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_H
#define SOCKET_H

#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

static const std::string HEADER_END = "\r\n\r\n";

class Socket
{
public:
	Socket();
	Socket(int fd);
	~Socket();

	int get();
	int bind_port(const std::string &host, const std::string &port);
	int listen_connection(int N);
	Socket accept_connection();

	std::string receive_all(int flag);
	ssize_t send_all(const char *buffer, size_t len, int flag);

	bool configure();

private:
	int fd_;
	int set_nonblocking();
	int set_reuseaddr();
};

#endif