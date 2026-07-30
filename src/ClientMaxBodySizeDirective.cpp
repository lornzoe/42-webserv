/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientMaxBodySizeDirective.cpp                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 00:00:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/27 12:44:39 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ClientMaxBodySizeDirective.hpp"
#include <exception>
#include <cctype>
#include <cstdlib>
#include <iostream>

namespace {
	std::size_t parseSize(const std::string& value)
	{
		if (value.empty())
			throw std::exception();

		std::string digits = value;
		std::size_t multiplier = 1;
		char suffix = value[value.size() - 1];

		if (std::isalpha(static_cast<unsigned char>(suffix)))
		{
			digits = value.substr(0, value.size() - 1);
			switch (std::tolower(static_cast<unsigned char>(suffix)))
			{
				case 'k': multiplier = 1024UL; break;
				case 'm': multiplier = 1024UL * 1024UL; break;
				case 'g': multiplier = 1024UL * 1024UL * 1024UL; break;
				default: throw std::exception();
			}
		}

		if (digits.empty())
			throw std::exception();
		for (std::string::const_iterator it = digits.begin(); it != digits.end(); ++it)
		{
			if (!std::isdigit(static_cast<unsigned char>(*it)))
				throw std::exception();
		}

		return static_cast<std::size_t>(std::strtoul(digits.c_str(), 0, 10)) * multiplier;
	}
}

ClientMaxBodySizeDirective::ClientMaxBodySizeDirective(TokenisedBlock::const_iterator& cit) : SimpleDirective(cit)
{
	if (args.size() != 2)
		throw std::exception(); // client_max_body_size takes exactly one argument

	bytes = parseSize(args[1]);
}

void ClientMaxBodySizeDirective::print(int depth) const
{
	for (int i = 0; i < depth; i++)
		std::cout << "  ";
	std::cout << "client_max_body_size " << bytes << ";" << std::endl;
}

std::size_t ClientMaxBodySizeDirective::getBytes() const { return bytes; }
