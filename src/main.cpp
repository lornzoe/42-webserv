/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ypua <ypua@student.42.singapore.sg>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/11 21:19:50 by lyanga            #+#    #+#             */
/*   Updated: 2026/09/06 23:14:13 by ypua             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "WSApp.hpp"

int main(int argc, char **argv)
{
	const char *DEFAULT_CONFIG_PATH = "configs/basic.conf";
	const char *config_file_path;

	if (argc != 2)
	{
		std::cout << "No configuration file specified." << std::endl;
		std::cout << "Usage: ./webserv [config_file]" << std::endl;
		std::cout << "Using default configuration: "
				  << DEFAULT_CONFIG_PATH << std::endl;
		config_file_path = DEFAULT_CONFIG_PATH;
	}
	else
	{
		config_file_path = argv[1];
	}
	WSApp ws;
	try
	{
		Config::init(config_file_path);
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
