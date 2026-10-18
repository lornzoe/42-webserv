#include "Pipe.hpp"

Pipe::Pipe()
	: _read_end(-1),
	  _write_end(-1)
{
}

bool Pipe::create()
{
	if (_read_end.get() != -1 || _write_end.get() != -1)
	{
		return false;
	}

	int fds[2];
	if (pipe(fds) == -1)
	{
		return false;
	}

	_read_end.resetFd(fds[0]);
	_write_end.resetFd(fds[1]);

	return true;
}

int Pipe::getReadFd() const
{
	return _read_end.get();
}

int Pipe::getWriteFd() const
{
	return _write_end.get();
}

void Pipe::closeRead()
{
	_read_end.closeFd();
}

void Pipe::closeWrite()
{
	_write_end.closeFd();
}

void Pipe::closeAll()
{
	closeRead();
	closeWrite();
}

Pipe &Pipe::operator=(const Pipe &other)
{
	(void)other;
	return *this;
}

bool Pipe::setNonBlockingRead()
{
	if (_read_end.get() == -1)
		return false;
	int flags = fcntl(_read_end.get(), F_GETFL, 0);
	return flags != -1 && fcntl(_read_end.get(), F_SETFL, flags | O_NONBLOCK) != -1;
}

bool Pipe::setNonBlockingWrite()
{
	if (_write_end.get() == -1)
		return false;
	int flags = fcntl(_write_end.get(), F_GETFL, 0);
	return flags != -1 && fcntl(_write_end.get(), F_SETFL, flags | O_NONBLOCK) != -1;
}

bool Pipe::setNonBlockingAll()
{
	return setNonBlockingRead() && setNonBlockingWrite();
}

const char *Pipe::PipeException::what() const throw()
{
	return "Pipe::PipeException.";
}
