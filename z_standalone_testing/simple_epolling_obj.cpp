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

#include "poller.hpp"

typedef struct s_fdCtx
{
	int	fd;
}	t_fdCtx;

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

	t_fdCtx		fdCtx = {listener_fd};
	Poller		poller;
	poller.add(listener_fd, EPOLLIN, &fdCtx);

	while (1)
	{
		int rdy_n = poller.wait(10000);
		struct epoll_event const *	rdy_events = poller.rdy_events();

		if (rdy_n == -1)
			return 1;
		for (int i = 0; i < rdy_n; ++i)
		{
			t_fdCtx	*	ctx = reinterpret_cast<t_fdCtx *>(rdy_events[i].data.ptr);
			if (rdy_events[i].events & EPOLLIN)
			{
				if (ctx->fd == listener_fd)
				{
					int client_fd = accept(listener_fd, NULL, NULL);
					fcntl(client_fd, F_SETFL, O_NONBLOCK);
					t_fdCtx	*	fdCtx = new t_fdCtx;
					fdCtx->fd = client_fd;
					poller.add(client_fd, EPOLLIN, fdCtx);
				}
				else
				{
					unsigned char	buf[42];
					ssize_t bytesrd = recv(ctx->fd, buf, 41, 0);
					buf[bytesrd] = '\0';
					std::cout << buf;
				}
			}
		}
	}
	return 0;
}
