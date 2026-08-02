#include "Poller.hpp"
#include "Server.hpp"
#include "w_logger.hpp"

#include <sys/socket.h>	//socket, bind, listen, accept, recv/send
#include <iostream>

int main(void)
{
	Poller		p;
	Server		s0("", 8080);

	p.add(s0.listener().fd(), EPOLLIN, &s0.listener().ectx());
	while (1)
	{
		int rdy_n = p.wait(10000);
		struct epoll_event const *	rdy_events = p.rdy_events();

		if (rdy_n == -1)
			return 1;

		for (int i = 0; i < rdy_n; ++i)
		{
			eventCtx *	ctx = static_cast<eventCtx *>(rdy_events[i].data.ptr);
			if (rdy_events[i].events & EPOLLIN)
			{
				if (ctx->type == SCK_LISTENER)
				{
					Listener &	lis = *(static_cast<Listener *>(ctx->owner));
					Server &	ser = lis.server();
					int client_fd = lis.welcome();
					if (client_fd == -1)
						continue ;
					ser.addClient(client_fd);
					p.add(client_fd, EPOLLIN, &ser.client(client_fd)->ectx());
				}
				else if (ctx->type == SCK_CLIENT)
				{
					Client &	cli = *(static_cast<Client *>(ctx->owner));
					if (cli.isClosing())
						continue;
					int ret = cli.recv1();
					if (ret <= 0)
						cli.toClose();
				}
			}
			if (rdy_events[i].events & EPOLLOUT)
			{
				if (ctx->type == SCK_CLIENT)
				{
					Client &	cli = *(static_cast<Client *>(ctx->owner));
					if (cli.isClosing())
						continue;
					int ret = cli.send1();
					(void)ret;
					if (!cli.isSending())
						p.mod(cli.fd(), EPOLLIN, &cli.ectx());
				}
			}
		}

		client_map_t::iterator i = s0.clients().begin();
		while (i != s0.clients().end())
		{
			Client & cli = i->second;
			if (cli.isClosing())
			{
				int cls_fd = cli.fd();
				++i;
				p.del(cls_fd);
				s0.rmClient(cls_fd);
				continue;
			}
			std::string const & inbox = cli.readInbox();
			if (inbox.size() >= 10 && !cli.isSending())
			{
				std::string tmp_resp = "Responding to inbox size 10\n";
				cli.req_resp(10, tmp_resp);
				p.mod(cli.fd(), EPOLLOUT | EPOLLIN, &cli.ectx());
			}
			++i;
		}
	}
	return 0;
}