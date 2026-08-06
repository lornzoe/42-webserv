/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ListenDirective.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 00:00:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/06 02:20:59 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ListenDirective.hpp"
#include <stdexcept>
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
		throw std::runtime_error("listen: expects exactly one argument (e.g. 'listen 127.0.0.1:8080;')");

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

	if (host.empty() || host == "*")
		host = "0.0.0.0";

	if (!isAllDigits(port_str))
		throw std::runtime_error("listen: port must be numeric (got '" + port_str + "')");

	// bound the digit count first so the conversion below cannot overflow
	if (port_str.size() > 5)
		throw std::runtime_error("listen: port must be between 1 and 65535 (got '" + port_str + "')");

	port = std::atoi(port_str.c_str());

	if (port < 1 || port > 65535)
		throw std::runtime_error("listen: port must be between 1 and 65535 (got '" + port_str + "')");
}

void ListenDirective::print(int depth) const
{
	for (int i = 0; i < depth; i++)
		std::cout << "  ";
	std::cout << "listen " << host << ":" << port << ";" << std::endl;
}

const std::string& ListenDirective::getHost() const { return host; }
int ListenDirective::getPort() const { return port; }
