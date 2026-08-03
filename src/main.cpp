#include "WSApp.hpp"
#include "Poller.hpp"
#include "Server.hpp"
#include "w_logger.hpp"

#include <sys/socket.h>	//socket, bind, listen, accept, recv/send
#include <iostream>

int main(void)
{
	WSApp		ws;
	ws.addServ("", 8080);
	ws.regisListeners();

	while (1)
	{
		epoll_res	rdy = ws.wait4events(10000);
		if (rdy.n == -1)
			return 1;

		for (int i = 0; i < rdy.n; ++i)
		{
			eventCtx *	ctx = static_cast<eventCtx *>(rdy.ep_res[i].data.ptr);
			switch (ctx->type)
			{
				case SCK_LISTENER:
					ws.hndl_Lis(ctx);
					break;
				case SCK_CLIENT:
					ws.hndl_Cli(ctx, rdy.ep_res[i].events);
					break;
				default:
					break;
			}
		}

		std::vector<Server *>::iterator it = ws.servBgn();
		while (it != ws.servEnd())
		{
			Server &	s = **it;
			client_map_t::iterator i = s.clients().begin();
			while (i != s.clients().end())
			{
				Client & cli = i->second;
				if (cli.isClosing())
				{
					int cls_fd = cli.fd();
					++i;
					ws.poller().del(cls_fd);
					s.rmClient(cls_fd);
					continue;
				}
				std::string const & inbox = cli.readInbox();
				if (inbox.size() >= 10 && !cli.isSending())
				{
					std::string tmp_resp = "Responding to inbox size 10\n";
					cli.req_resp(10, tmp_resp);
					ws.poller().mod(cli.fd(), EPOLLOUT | EPOLLIN, &cli.ectx());
				}
				++i;
			}
			++it;
		}
	}
	return 0;
}