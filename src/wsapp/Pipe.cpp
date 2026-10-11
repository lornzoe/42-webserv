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
