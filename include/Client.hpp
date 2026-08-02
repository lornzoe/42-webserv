#ifndef CLIENT_H
# define CLIENT_H

#include "w_eventCtx.hpp"

# include <unistd.h>
# include <string>

class Server;

enum cli_status {
	SENDING = 1 << 0,
	CLOSING = 1 << 1
};

class Client
{
private:
	Server *		_server;
	int				_fd;
	eventCtx		_eCtx;

	int				_status;

	std::string		_inbox;
	std::string		_outBox;
	int				_outPend;
	int				_outCursor;

public:
	Client();
	~Client();

	//faux copy constructor & assignment equivalent to default
	Client(Client const & other) : _fd(-1) { (void)other; }
	Client &	operator=(Client const &other) { if (this != &other) _fd = -1; return *this; }

	Server &		server()	{ return *_server; }
	int				fd() 		{ return _fd; }
	eventCtx &		ectx()		{ return _eCtx; }

	void			initClient(Server &server, int fd);
	
	std::string const &		readInbox() const { return _inbox; }
	bool					isSending() const { return _status & SENDING; }
	bool					isClosing() const { return _status & CLOSING; }
	void					toClose() { _status |= CLOSING; }

	ssize_t			recv1();
	void			req_resp(size_t req_offset, std::string const &resp);
	ssize_t			send1();
};

#endif
