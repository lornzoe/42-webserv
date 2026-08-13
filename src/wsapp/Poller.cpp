#include "Poller.hpp"
#include <iostream>
#include <stdexcept>
#include <cstring>		//sterror
#include <cerrno>
#include <unistd.h>		//close

// OCF ------------------------------------------------------------------------

Poller::Poller() : _epfd(epoll_create(1)) {
	if (_epfd == -1)
		throw std::runtime_error(strerror(errno));
}

Poller::~Poller() {
	close(_epfd);
}

// ----------------------------------------------------------------------------

//TO-DO proper error handling

int		Poller::add(int fd, uint32_t events, void *data) {
	struct epoll_event ev = {};
	ev.events = events;
	ev.data.ptr = data;
	return epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &ev);
}

int		Poller::mod(int fd, uint32_t events, void *data) {
	struct epoll_event ev = {};
	ev.events = events;
	ev.data.ptr = data;
	return epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &ev);
}

int		Poller::del(int fd) {
	return epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, NULL);
}

/** Returns
 * >0	: number of ready events
 * ==0	: timeout without any ready events
 * -1	: error, sets errno
 */
int		Poller::wait() {
	return epoll_wait(_epfd, _rdy_events, _max_events, _timeout_ms);
}

// returns ptr to array of ready epoll_events, rdy_num is returned from call to wait
struct epoll_event const *	Poller::rdy_events() const {
	return _rdy_events;
}
