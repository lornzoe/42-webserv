/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigChecker.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/02 06:33:02 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/10 01:37:57 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <exception>

#include "Config.hpp"

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cerr << "Usage: ./config_checker <config_file>" << std::endl;
		return 1;
	}

	try
	{
		Config::init(argv[1]);
		const Config& c = Config::getInstance();
		std::cout << "== tokenised config:" << std::endl;
		c.printConfig();
		std::cout << "== directives:" << std::endl;
		c.printDirectives();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
