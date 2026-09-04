/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorPageDirective.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 00:00:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/06 02:21:07 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ErrorPageDirective.hpp"
#include "HttpStat.hpp"
#include <stdexcept>
#include <cctype>
#include <cstdlib>
#include <iostream>

ErrorPageDirective::ErrorPageDirective(TokenisedBlock::const_iterator& cit) : SimpleDirective(cit)
{
	if (args.size() < 3) // name + at least one code + a path
		throw std::runtime_error("error_page: expects at least one status code and a path (e.g. 'error_page 404 /error.html;')");

	for (std::size_t i = 1; i + 1 < args.size(); i++)
	{
		const std::string& code_str = args[i];
		if (code_str.empty())
			throw std::runtime_error("error_page: status code must not be empty");
		for (std::string::const_iterator it = code_str.begin(); it != code_str.end(); ++it)
		{
			if (!std::isdigit(static_cast<unsigned char>(*it)))
				throw std::runtime_error("error_page: status code must be numeric (got '" + code_str + "')");
		}
		if (code_str.size() > 3)
			throw std::runtime_error("error_page: status code must be between 300 and 599 (got '" + code_str + "')");

		int code = std::atoi(code_str.c_str());

		// nginx restricts error_page to 300-599; reject anything outside that
		// range, or a code we have no reason phrase for.
		if (code < 300 || code > 599)
			throw std::runtime_error("error_page: status code must be between 300 and 599 (got '" + code_str + "')");
		if (!HttpStat::isKnown(code))
			throw std::runtime_error("error_page: unknown HTTP status code '" + code_str + "'");

		codes.push_back(code);
	}
	path = args.back();
}

void ErrorPageDirective::print(int depth) const
{
	for (int i = 0; i < depth; i++)
		std::cout << "  ";
	std::cout << "error_page";
	for (std::size_t i = 0; i < codes.size(); i++)
		std::cout << " " << codes[i];
	std::cout << " " << path << ";" << std::endl;
}

const std::vector<int>& ErrorPageDirective::getCodes() const { return codes; }
const std::string& ErrorPageDirective::getPath() const { return path; }
