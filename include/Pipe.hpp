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

private:
	FileDescriptor _read_end;
	FileDescriptor _write_end;

	Pipe(const Pipe &other);
	Pipe &operator=(const Pipe &other);
};

#endif
