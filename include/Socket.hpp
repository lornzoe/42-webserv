/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ypua <ypua@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/21 19:30:23 by ypua              #+#    #+#             */
/*   Updated: 2026/07/21 20:08:04 by ypua             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_H
#define SOCKET_H

#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

class Socket
{
public:
	Socket();
	Socket(int fd);
	~Socket();

	int get();
	int bind_port(unsigned long port);
	int listen_connection(int N);
	Socket accept_connection();

	ssize_t receive(char *buffer, size_t len, int flag);
	ssize_t send_all(const char *buffer, size_t len, int flag);

private:
	int fd_;
};

#endif