#ifndef EVENTCTX_H
# define EVENTCTX_H

enum e_type {
	SCK_LISTENER,
	SCK_CLIENT,
	CGI_IN,
	CGI_OUT
};

struct EventCtx
{ 
	int		fd;
	e_type	type;
	void *	owner;
};

#endif
