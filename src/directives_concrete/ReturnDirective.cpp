/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ReturnDirective.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 00:00:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/06 02:18:54 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ReturnDirective.hpp"
#include "HttpStat.hpp"
#include <stdexcept>
#include <cctype>
#include <cstdlib>
#include <iostream>

ReturnDirective::ReturnDirective(TokenisedBlock::const_iterator& cit) : SimpleDirective(cit)
{
	if (args.size() != 2 && args.size() != 3)
		throw std::runtime_error("return: expects a status code and an optional body/url (e.g. 'return 301 /new;')");

	const std::string& code_str = args[1];
	if (code_str.empty())
		throw std::runtime_error("return: status code must not be empty");
	for (std::string::const_iterator it = code_str.begin(); it != code_str.end(); ++it)
	{
		if (!std::isdigit(static_cast<unsigned char>(*it)))
			throw std::runtime_error("return: status code must be numeric (got '" + code_str + "')");
	}
	if (code_str.size() > 3)
		throw std::runtime_error("return: unknown HTTP status code '" + code_str + "'");

	code = std::atoi(code_str.c_str());

	if (!HttpStat::isKnown(code))
		throw std::runtime_error("return: unknown HTTP status code '" + code_str + "'");

	if (args.size() == 3)
		body = args[2];
}

void ReturnDirective::print(int depth) const
{
	for (int i = 0; i < depth; i++)
		std::cout << "  ";
	std::cout << "return " << code;
	if (!body.empty())
		std::cout << " " << body;
	std::cout << ";" << std::endl;
}

int ReturnDirective::getCode() const { return code; }
const std::string& ReturnDirective::getBody() const { return body; }
