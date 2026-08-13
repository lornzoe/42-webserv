#ifndef POLLER_H
# define POLLER_H

#include <sys/epoll.h>

class Poller
{
private:
	static int const	_max_events = 128;
	static int const	_timeout_ms = 1000;

	int					_epfd;
	struct epoll_event	_rdy_events[_max_events];

	Poller &	operator=(Poller const &other);
	Poller(Poller const &other);

public:
	Poller();
	~Poller();

	int		add(int fd, uint32_t events, void *data);
	int		mod(int fd, uint32_t events, void *data);
	int		del(int fd);

	int		wait();
	struct epoll_event const *	rdy_events() const;
};

#endif
