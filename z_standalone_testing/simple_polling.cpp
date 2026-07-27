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

	struct pollfd	pfd[1024];

	pfd[0].fd = listener_fd;
	pfd[0].events = POLLIN;

	int nfds = 1;
	while (1)
	{
		poll(pfd, nfds, 10);
		for (int i = 0; i < nfds; ++i)
		{
			unsigned char	buf[42];
			if (pfd[i].revents == POLLIN)
			{
				if (pfd[i].fd == listener_fd)
				{
					pfd[nfds].fd = accept(listener_fd, NULL, NULL);
					pfd[nfds].events = POLLIN;
					++nfds;
				}
				else
				{
					ssize_t bytesrd = recv(pfd[i].fd, buf, 41, 0);
					buf[bytesrd] = '\0';
					std::cout << buf;
				}
			}
		}
	}
	return 0;
}
