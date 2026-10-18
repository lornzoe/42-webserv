#include "Listener.hpp"
#include "Server.hpp"
#include "Utils.hpp"
#include "w_logger.hpp"
#include <iostream>

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
	Utils::safeClose(_fd);
}

// An empty host (empty str "") will bind to the wildcard address aka 0.0.0.0
Listener::Listener(Server &server, std::string const &host, int port) 
: _server(server), _fd(-1)
{
	std::string			str_port = Utils::ft_itoa(port);

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
				Utils::safeClose(_fd);
				cur = cur->ai_next;
			}
			else {
				int tmp_errno = errno;
				Utils::safeClose(_fd);
				freeaddrinfo(res);
				throw std::runtime_error(strerror(tmp_errno));
			}
		}
		else break;
	}
	freeaddrinfo(res);

	_eCtx.fd = _fd;
	_eCtx.type = SCK_LISTENER;
	_eCtx.owner = this;
	LOG_INFO("Listener fd " << _fd << " created on Server " << _server.name());
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

static std::string	IPv4_addrStr(unsigned int addr)
{
	std::stringstream ss;
	ss	<< ((addr >> 24) & 0xFF) << "."
		<< ((addr >> 16) & 0xFF) << "."
		<< ((addr >> 8) & 0xFF) << "."
		<< ((addr >> 0) & 0xFF);
	return ss.str();
}

int				Listener::welcome(remote_info &rinfo)
{
	int client_fd = -1;

	struct sockaddr_in			sin;
	socklen_t					sin_len = sizeof(sin);

	client_fd = accept(_fd, reinterpret_cast<sockaddr *>(&sin), &sin_len);
	if (client_fd == -1)
	{
		if (errno != EAGAIN)
			LOG_WARN(strerror(errno));
		return -1;
	}

	if (noBlock(client_fd) == -1)
	{
		LOG_WARN(strerror(errno));
		Utils::safeClose(client_fd);
		return -1;
	}

	rinfo.addr = IPv4_addrStr(ntohl(sin.sin_addr.s_addr));
	rinfo.port = ntohs(sin.sin_port);

	return client_fd;
}
