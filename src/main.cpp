#include <iostream>
#include <exception>

#include "Config.hpp"
#include "ServerDirective.hpp"
#include "ListenDirective.hpp"
#include "WSApp.hpp"
#include "FileDescriptor.hpp"
#include "Socket.hpp"
#include <vector>

#include <sstream>
#include <sys/stat.h>

// TODO: Parse HTTP request into Method, Request URL, HTTP Version + Host,
// Headers: Content-Length, Transfer-Encoding and Content-Type, Body

// Example:
// GET /favicon.ico HTTP/1.1
// Host: localhost:8080
// User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:142.0) Gecko/20100101 Firefox/142.0
// Accept: image/avif,image/webp,image/png,image/svg+xml,image/*;q=0.8,*/*;q=0.5
// Accept-Language: en-US,en;q=0.5
// Accept-Encoding: gzip, deflate, br, zstd
// Connection: keep-alive
// Referer: http://localhost:8080/
// Sec-Fetch-Dest: image
// Sec-Fetch-Mode: no-cors
// Sec-Fetch-Site: same-origin
// Priority: u=6

// Finds the first "listen" directive under the first "server" block in the
// parsed config. Returns false if none is found (caller should fall back).
static bool findFirstListen(const Config& config, std::string& host, int& port)
{
	const std::vector<Directive *>& top = config.getDirectives();
	for (std::size_t i = 0; i < top.size(); i++)
	{
		const ServerDirective* server = dynamic_cast<const ServerDirective*>(top[i]);
		if (!server)
			continue;

		const std::vector<Directive *>& children = server->getChildren();
		for (std::size_t j = 0; j < children.size(); j++)
		{
			const ListenDirective* listen = dynamic_cast<const ListenDirective*>(children[j]);
			if (listen)
			{
				host = listen->getHost();
				port = listen->getPort();
				std::cout << "[webserv] connecting to host "
						<< host << " on port " << port << std::endl;
				return true;
			}
		}
	}
	return false;
}

int main(int argc, char** argv, char **evp)
{
	(void)evp;
	if (argc != 2)
	{
		std::cerr << "Usage: ./webserv <config_file>" << std::endl;
		return 1;
	}

	std::string host;
	int port = 8080;
	try
	{
		Config c(argv[1]);
		std::cout << "[webserv] Config() completed." << std::endl;
		// c.printConfig();
		// c.printDirectives();

		if (!findFirstListen(c, host, port))
		{
			std::cerr << "Warning: no 'listen' directive found in config, "
						 "defaulting to port 8080" << std::endl;
			host = "";
			port = 8080;
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	std::cout << "[webserv] running server now." << std::endl;	
	WSApp	ws;
	ws.addServ(host, port);
	return ws.run();
}
