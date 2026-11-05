/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ReturnDirective.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 00:00:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/11/05 16:27:02 by lyanga           ###   ########.fr       */
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

	bool isRedirect = (code == 301 || code == 302 || code == 303
					|| code == 307 || code == 308);
	// reject redirect codes that are not supported here (300, 304, 305, 306)
	if (code >= 300 && code < 400 && !isRedirect)
		throw std::runtime_error("return: unsupported redirect code " + code_str + " (only 301, 302, 303, 307, 308 are supported)");
	// reject redirect codes without a destination url
	// e.g. reject 302;
	if (isRedirect && body.empty())
		throw std::runtime_error("return: redirect code " + code_str + " requires a URL (e.g. 'return " + code_str + " /url/;')");

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
