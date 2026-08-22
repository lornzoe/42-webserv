#ifndef CLIENT_H
#define CLIENT_H

#include "ServerDirective.hpp"
#include "w_eventCtx.hpp"
#include "HttpRequest.hpp"

#include <unistd.h>
#include <string>

class Server;

enum cli_status
{
	SENDING = 1 << 0,
	CLOSING = 1 << 1
};

class Client
{
private:
	Server *_server;
	int _fd;
	eventCtx _eCtx;

	int _status;

	std::string _inbox;
	std::string _outBox;
	int _outPend;
	int _outCursor;

public:
	Client();
	~Client();
	// faux copy constructor & assignment equivalent to default
	Client(Client const &other);
	Client &operator=(Client const &other);

	void initClient(Server &server, int fd);

	ServerDirective const &servDir() const;
	Server &server() { return *_server; }
	int fd() { return _fd; }
	eventCtx &ectx() { return _eCtx; }

	std::string const &readInbox() const { return _inbox; }
	bool isStat(int flag) const { return ((_status & flag) == flag); }
	void addStat(int flag) { _status |= flag; }
	void rmStat(int flag) { _status &= ~flag; }

	ssize_t recv1();
	void send_response(ssize_t req_offset, std::string const &resp);
	ssize_t send1();

	void process_request(ParseResult const &result);
};

#endif
