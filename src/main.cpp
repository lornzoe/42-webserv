/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/11 21:19:50 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/27 12:44:27 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include <iostream>
#include <exception>

#include "Config.hpp"

int main(int argc, char** argv) {
	
	if (argc != 2)
	{
		std::cerr << "Usage: ./webserv <config_file>" << std::endl;
		return 1;
	}

	try
	{
		Config c(argv[1]);
		c.printConfig();
		c.printDirectives();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}