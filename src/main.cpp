#include "Poller.hpp"
#include "Server.hpp"

#include <sys/socket.h>	//socket, bind, listen, accept, recv/send

#include <iostream>


int main(void)
{
	Poller		p;
	Server		s("", 8080);

	p.add(s.listener().fd(), EPOLLIN, s.listener().ctx());
	while (1)
	{
		int rdy_n = p.wait(10000);
		struct epoll_event const *	rdy_events = p.rdy_events();

		if (rdy_n == -1)
			return 1;

		for (int i = 0; i < rdy_n; ++i)
		{
			EventCtx *	ctx = reinterpret_cast<EventCtx *>(rdy_events[i].data.ptr);
			if (rdy_events[i].events & EPOLLIN)
			{
				if (ctx->type == SCK_LISTENER)
				{
					int client_fd = s.listener().regisClient();
					if (client_fd == -1)
						continue;
					s.addClient(client_fd);
					p.add(client_fd, EPOLLIN, s.getClient(client_fd).ctx());
				}
				else if (ctx->type == SCK_CLIENT)
				{
					unsigned char	buf[42];
					Client *client = static_cast<Client *>(ctx->owner);
					ssize_t bytesrd = recv(client->fd(), buf, 41, 0);
					buf[bytesrd] = '\0';
					std::cout << buf;
				}
			}
		}
	}
	return 0;
}