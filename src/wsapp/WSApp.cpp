#include "WSApp.hpp"
#include "FileDescriptor.hpp"
#include "w_logger.hpp"

#include <string>
#include <sstream>
#include <sys/stat.h>

volatile sig_atomic_t	WSApp::g_shutdownReq = 0;
static const std::string HEADER_END = "\r\n\r\n";

// OCF ------------------------------------------------------------------------

WSApp::WSApp() {}

WSApp::~WSApp()
{
	for (std::vector<Server *>::iterator it = _servs.begin();
		it != _servs.end(); ++it)
	delete *it;
}

// ----------------------------------------------------------------------------

static int	fill_tmp_resp(std::string &resp)
{
	FileDescriptor file("html/index.html");
	if (file.get() == -1)
		return 1;

	struct stat st;
	if (stat("html/index.html", &st) == -1)
		return 1;

	std::vector<char> body(st.st_size);

	ssize_t total = 0;
	while (total < st.st_size)
	{
		ssize_t bytes = read(file.get(),
							 &body[total],
							 st.st_size - total);

		if (bytes <= 0)
			return 1;

		total += bytes;
	}

	std::stringstream ss;
	ss << total;
	std::string length = ss.str();

	std::string header =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: " +
		length + HEADER_END;

	resp = header;
	resp.append(body.begin(), body.end());
	return 0;
}

int		WSApp::run()
{
	std::string tmp_resp;
	if (fill_tmp_resp(tmp_resp) == 1)
	{
		std::cerr << "i give up" << std::endl;
		return 1;
	}

	regisListeners();
	while (1)
	{
		epoll_res	rdy = wait(10000);
		if (rdy.n == -1)
			return 1;

		for (int i = 0; i < rdy.n; ++i)
		{
			eventCtx *	ctx = static_cast<eventCtx *>(rdy.ep_ev[i].data.ptr);
			switch (ctx->type)
			{
				case SCK_LISTENER:
					hndl_Lis(ctx);
					break;
				case SCK_CLIENT:
					hndl_Cli(ctx, rdy.ep_ev[i].events);
					break;
				default:
					break;
			}
		}

		std::vector<Server *>::iterator it = _servs.begin();
		while (it != _servs.end())
		{
			Server &	s = **it;
			client_map_t::iterator i = s.clients().begin();
			while (i != s.clients().end())
			{
				Client & cli = i->second;
				if (cli.isStat(CLOSING))
				{
					int cls_fd = cli.fd();
					++i;
					_pol.del(cls_fd);
					s.rmClient(cls_fd);
					continue;
				}
				std::string const & inbox = cli.readInbox();
				if (inbox.size() > 0 && !cli.isStat(SENDING))
				{
					cli.tmp_req_for_resp(-1, tmp_resp);
					_pol.mod(cli.fd(), EPOLLOUT | EPOLLIN, &cli.ectx());
				}
				++i;
			}
			++it;
		}
	}
	return 0;
}

void	WSApp::addServ(std::string const &host, int port)
{
	Server *	new_serv = new Server(host, port);
	_servs.push_back(new_serv);
}

// Private --------------------------------------------------------------------

// to check return from epoll add
void	WSApp::regisListeners()
{
	for (std::vector<Server *>::iterator it = _servs.begin();
		it != _servs.end(); ++it)
	{
		Listener &	lis = (*it)->listener();
		_pol.add(lis.fd(), EPOLLIN, &lis.ectx());
	}
}

epoll_res	WSApp::wait(int timeout_ms)
{
	int rdy_n = _pol.wait(timeout_ms);
	struct epoll_event const *	rdy_events = _pol.rdy_events();

	epoll_res res = {rdy_n, rdy_events};
	return res;
}

int		WSApp::hndl_Lis(eventCtx *ctx)
{
	Listener &	lis = *(static_cast<Listener *>(ctx->owner));
	Server &	ser = lis.server();

	int client_fd = lis.welcome();
	if (client_fd == -1)
		return -1;

	Client * 	cli = ser.addClient(client_fd);
	if (!cli)
		return -1;

	return _pol.add(client_fd, EPOLLIN, &cli->ectx());
	//rm client from server if add fails...
}

int		WSApp::hndl_Cli(eventCtx *ctx, uint32_t events)
{
	Client &	cli = *(static_cast<Client *>(ctx->owner));

	if (events & EPOLLIN)
	{
		if (cli.isStat(CLOSING))
			return 0;
		int ret = cli.recv1();
		if (ret <= 0)
			cli.addStat(CLOSING);
	}

	if (events & EPOLLOUT)
	{
		if (cli.isStat(CLOSING))
			return 0;
		int ret = cli.send1();
		(void)ret;
		if (!cli.isStat(SENDING))
			_pol.mod(cli.fd(), EPOLLIN, &cli.ectx());
	}

	return 0;
}
