/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ypua <ypua@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/11 21:19:50 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/13 20:12:29 by ypua             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "WSApp.hpp"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cerr << "Usage: ./webserv <config_file>" << std::endl;
		return 1;
	}
	WSApp	ws;
	try
	{
		Config::init(argv[1]);
		std::cout << "[webserv] Config() completed." << std::endl;
		ws.ConfigInit(Config::getInstance());
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	std::cout << "[webserv] running server now." << std::endl;
	ws.run();
	return 0;
}
