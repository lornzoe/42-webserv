/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/11 21:19:50 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/14 07:41:15 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include <iostream>

#include "Config.hpp"

int main(int argc, char** argv) {
	
	if (argc != 2)
	{
		std::cerr << "Usage: ./webserv <config_file>" << std::endl;
		return 1;
	}

	// Temporarily here to silence the compiler.
	(void)argc;
	(void)argv;

	// Main here.
	// std::cout << "This is where the webserv would've been, if I had one!"
	// 	<< std::endl;

	Config c(argv[1]);
	c.printConfig();
	
	return 0;
}