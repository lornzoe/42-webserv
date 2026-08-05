#include "WSApp.hpp"
#include "Poller.hpp"
#include "Server.hpp"
#include "w_logger.hpp"

#include <sys/socket.h>	//socket, bind, listen, accept, recv/send
#include <iostream>

int main(int ac, char *av[], char *evp[])
{
	WSApp		ws;

	(void)ac;
	(void)av;
	(void)evp;

	ws.addServ("", 8080);
	ws.run();

	return 0;
}