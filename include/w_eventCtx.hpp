#ifndef W_EVENTCTX_H
# define W_EVENTCTX_H

# include <cstdlib>

enum e_type {
	NONE,
	SCK_LISTENER,
	SCK_CLIENT,
	CGI_IN,
	CGI_OUT
};

struct eventCtx
{ 
	int		fd;
	e_type	type;
	void *	owner;

	eventCtx() : fd(-1), type(NONE), owner(NULL) {}
};

#endif
