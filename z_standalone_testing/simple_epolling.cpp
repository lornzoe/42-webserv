#include <sys/socket.h>	//socket, bind, listen, accept, recv/send
#include <sys/types.h>	

#include <netdb.h>		//for getaddrinfo structs
#include <fcntl.h>		//socket config

#include <poll.h>		//basic polling
#include <sys/epoll.h>	//epoll

#include <cstdlib>
#include <iostream>
#include <arpa/inet.h>

#include <sstream>
#include <unistd.h>

int main(void)
{
	struct addrinfo		hints = {};
	struct addrinfo *	res = NULL;

	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;	//for intended use with bind()

	getaddrinfo(NULL, "8080", &hints, &res);
	
	int listener_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	
	//socket configuration
	int fd_flag = fcntl(listener_fd, F_GETFL, 0);
	fcntl(listener_fd, F_SETFL, fd_flag | O_NONBLOCK);
	int optVal = 1;
	setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));

	//bind to address and start listening
	bind(listener_fd, res->ai_addr, res->ai_addrlen);
	listen(listener_fd, 1024);

	int epfd = epoll_create(1);
	struct epoll_event	ev = {};
	ev.events = EPOLLIN;
	ev.data.fd = listener_fd;
	epoll_ctl(epfd, EPOLL_CTL_ADD, listener_fd, &ev);

	while (1)
	{
		struct epoll_event re[128] = {};
		int rdy_n = epoll_wait(epfd, re, 128, 10000);
		for (int i = 0; i < rdy_n; ++i)
		{
			unsigned char	buf[42];
			if (re[i].events & EPOLLIN)
			{
				if (re[i].data.fd == listener_fd)
				{
					int client_fd = accept(listener_fd, NULL, NULL);
					int fd_flag = fcntl(client_fd, F_GETFL, 0);
					fcntl(client_fd, F_SETFL, fd_flag | O_NONBLOCK);
					struct epoll_event	new_ev = {};
					new_ev.events = EPOLLIN;
					new_ev.data.fd = client_fd;
					epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &new_ev);
				}
				else
				{
					ssize_t bytesrd = recv(re[i].data.fd, buf, 41, 0);
					buf[bytesrd] = '\0';
					std::cout << buf;
				}
			}
		}
	}
	return 0;
}
