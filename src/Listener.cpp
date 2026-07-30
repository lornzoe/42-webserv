#include "Listener.hpp"
#include "WUtils.hpp"

#include <sys/socket.h>	//socket, bind, listen, accept, recv/send
#include <sys/types.h>
#include <netdb.h>		//for getaddrinfo structs
#include <fcntl.h>		//socket config

#include <stdexcept>
#include <cstring>		//sterror
#include <cerrno>

#include <sstream>

// OCF ------------------------------------------------------------------------

Listener::~Listener() {
	WUtils::safeClose(_fd);
}

// An empty host (empty str "") will bind to the wildcard address aka 0.0.0.0
Listener::Listener(Server const &server, std::string const &host, int port) 
: _fd(-1), _server(server)
{
	std::stringstream	ss;
	std::string			str_port;
	ss << port;
	str_port = ss.str();

	struct addrinfo		hints = {};
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	struct addrinfo *	res = NULL;

	int ret = getaddrinfo(	host.empty() ? NULL : host.c_str(),
							str_port.c_str(), &hints, &res);
	if (ret != 0)
		throw std::runtime_error(gai_strerror(ret));

	struct addrinfo *	cur = res;
	while (cur) {
		if ((_fd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol)) == -1
			|| noBlock(_fd) == -1 || reuseAddr(_fd) == -1
			|| bind(_fd, cur->ai_addr, cur->ai_addrlen) == -1
			|| listen(_fd, 1024) == -1)
		{
			if (cur->ai_next != NULL) {
				WUtils::safeClose(_fd);
				cur = cur->ai_next;
			}
			else {
				int tmp_errno = errno;
				WUtils::safeClose(_fd);
				freeaddrinfo(res);
				throw std::runtime_error(strerror(tmp_errno));
			}
		}
		else break;
	}
	freeaddrinfo(res);

	_ctx.fd = _fd;
	_ctx.type = SCK_LISTENER;
	_ctx.owner = this;
}

// Private --------------------------------------------------------------------

int		Listener::noBlock(int fd) {
	return fcntl(fd, F_SETFL, O_NONBLOCK);
}

int		Listener::reuseAddr(int fd) {
	int optVal = 1;
	return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));
}

// ----------------------------------------------------------------------------

int				Listener::fd() {
	return _fd;
}

EventCtx *		Listener::ctx() {
	return &_ctx;
}

int				Listener::regisClient() {
	int client_fd = -1;
	if ((client_fd = accept(_fd, NULL, NULL)) == -1
		|| noBlock(client_fd) == -1)
	{
		WUtils::safeClose(client_fd);
		return -1;
	}
	return client_fd;
}
