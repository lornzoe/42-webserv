/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ListenDirective.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 00:00:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/27 12:44:30 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ListenDirective.hpp"
#include <exception>
#include <cctype>
#include <cstdlib>
#include <iostream>

namespace {
	bool isAllDigits(const std::string& s)
	{
		if (s.empty())
			return false;
		for (std::string::const_iterator it = s.begin(); it != s.end(); ++it)
		{
			if (!std::isdigit(static_cast<unsigned char>(*it)))
				return false;
		}
		return true;
	}
}

ListenDirective::ListenDirective(TokenisedBlock::const_iterator& cit) : SimpleDirective(cit), host("0.0.0.0")
{
	if (args.size() != 2)
		throw std::exception(); // listen takes exactly one argument

	const std::string& value = args[1];
	std::size_t colon = value.find(':');
	std::string port_str;

	if (colon != std::string::npos)
	{
		host = value.substr(0, colon);
		port_str = value.substr(colon + 1);
	}
	else
		port_str = value;

	if (!isAllDigits(port_str))
		throw std::exception(); // port must be numeric

	port = std::atoi(port_str.c_str());
}

void ListenDirective::print(int depth) const
{
	for (int i = 0; i < depth; i++)
		std::cout << "  ";
	std::cout << "listen " << host << ":" << port << ";" << std::endl;
}

const std::string& ListenDirective::getHost() const { return host; }
int ListenDirective::getPort() const { return port; }
