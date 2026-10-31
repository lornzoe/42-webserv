#ifndef CLIENT_H
#define CLIENT_H

#include "ServerDirective.hpp"
#include "w_eventCtx.hpp"
#include "HttpRequest.hpp"
#include "ReqProc.hpp"

#include <unistd.h>
#include <string>
#include <ctime>

class Server;

enum cli_status
{
	SENDING = 1 << 0,
	CLOSING = 1 << 1,
	NEWSESH = 1 << 8
};

class Client
{
private:
	Server *_server;
	int _fd;
	eventCtx _eCtx;

	int 	_status;
	
	size_t	_maxBodySize;
	std::string _inbox;
	std::string _outBox;
	int _outPend;
	int _outCursor;

	time_t _last_activity_time;
	sesh_data *		_sdata;

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

	bool isStat(int flag) const { return ((_status & flag) == flag); }
	void addStat(int flag) { _status |= flag; }
	void rmStat(int flag) { _status &= ~flag; }

	size_t	maxBodySize() const { return _maxBodySize; }
	std::string const &readInbox() const { return _inbox; }

	ssize_t recv1();
	ReqProc::status	process_request(ParseResult &result);

	void comsume_inbox(ssize_t req_offset);
	void send_response(std::string const &resp);
	ssize_t send1();

	time_t get_last_activity_time();
	void refresh_last_activity_time();

};

#endif
