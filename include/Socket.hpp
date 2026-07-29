/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ypua <ypua@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/21 19:30:23 by ypua              #+#    #+#             */
/*   Updated: 2026/07/29 20:57:27 by ypua             ###   ########.fr       */
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
	int bind_port(const std::string &port);
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