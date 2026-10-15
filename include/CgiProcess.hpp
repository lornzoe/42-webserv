#ifndef CGIPROCESS_H
#define CGIPROCESS_H

#include <Pipe.hpp>

class CgiProcess
{
public:
	CgiProcess();

	void start();

	class ForkException : public std::exception
	{
	public:
		virtual const char *what() const throw();
	};

private:
	Pipe _parent_to_child;
	Pipe _child_to_parent;
};

#endif