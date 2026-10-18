#ifndef PIPE_H
#define PIPE_H

#include <unistd.h>
#include "FileDescriptor.hpp"

class Pipe
{
public:
	Pipe();

	bool create();

	int getReadFd() const;
	int getWriteFd() const;

	void closeRead();
	void closeWrite();
	void closeAll();

	bool setNonBlockingRead();
	bool setNonBlockingWrite();
	bool setNonBlockingAll();

	class PipeException : public std::exception
	{
	public:
		virtual const char *what() const throw();
	};

private:
	FileDescriptor _read_end;
	FileDescriptor _write_end;

	Pipe(const Pipe &other);
	Pipe &operator=(const Pipe &other);
};

#endif
