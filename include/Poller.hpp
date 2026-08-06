#ifndef POLLER_H
# define POLLER_H

#include <sys/epoll.h>

class Poller
{
private:
	Poller &	operator=(Poller const &other);
	Poller(Poller const &other);

	static int const	_max_events = 128;

	int					_epfd;
	struct epoll_event	_rdy_events[_max_events];

public:
	Poller();
	~Poller();

	int		add(int fd, uint32_t events, void *data);
	int		mod(int fd, uint32_t events, void *data);
	int		del(int fd);

	int		wait(int timeout_ms);
	struct epoll_event const *	rdy_events() const;
};

#endif
