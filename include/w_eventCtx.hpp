#ifndef W_EVENTCTX_H
# define W_EVENTCTX_H

enum e_type {
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
};

#endif
