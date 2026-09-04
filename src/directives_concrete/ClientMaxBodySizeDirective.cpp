/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientMaxBodySizeDirective.cpp                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 00:00:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/02 07:17:34 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ClientMaxBodySizeDirective.hpp"
#include <stdexcept>
#include <cctype>
#include <cstdlib>
#include <iostream>

namespace {
	std::size_t parseSize(const std::string& value)
	{
		if (value.empty())
			throw std::runtime_error("client_max_body_size: value must not be empty");

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
				default: throw std::runtime_error(
					"client_max_body_size: unknown size suffix '" + std::string(1, suffix) + "' (expected k, m, or g)");
			}
		}

		if (digits.empty())
			throw std::runtime_error(
				"client_max_body_size: missing numeric value before suffix '" + std::string(1, suffix) + "'");
		for (std::string::const_iterator it = digits.begin(); it != digits.end(); ++it)
		{
			if (!std::isdigit(static_cast<unsigned char>(*it)))
				throw std::runtime_error("client_max_body_size: value must be numeric (got '" + value + "')");
		}

		return static_cast<std::size_t>(std::strtoul(digits.c_str(), 0, 10)) * multiplier;
	}
}

ClientMaxBodySizeDirective::ClientMaxBodySizeDirective(TokenisedBlock::const_iterator& cit) : SimpleDirective(cit)
{
	if (args.size() != 2)
		throw std::runtime_error("client_max_body_size: expects exactly one argument (e.g. 'client_max_body_size 10m;')");

	bytes = parseSize(args[1]);
}

void ClientMaxBodySizeDirective::print(int depth) const
{
	for (int i = 0; i < depth; i++)
		std::cout << "  ";
	std::cout << "client_max_body_size " << bytes << ";" << std::endl;
}

std::size_t ClientMaxBodySizeDirective::getBytes() const { return bytes; }
