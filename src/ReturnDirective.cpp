/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ReturnDirective.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 00:00:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/27 12:44:25 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ReturnDirective.hpp"
#include <exception>
#include <cctype>
#include <cstdlib>
#include <iostream>

ReturnDirective::ReturnDirective(TokenisedBlock::const_iterator& cit) : SimpleDirective(cit)
{
	if (args.size() != 2 && args.size() != 3)
		throw std::exception(); // return takes a code and an optional body/url

	const std::string& code_str = args[1];
	if (code_str.empty())
		throw std::exception();
	for (std::string::const_iterator it = code_str.begin(); it != code_str.end(); ++it)
	{
		if (!std::isdigit(static_cast<unsigned char>(*it)))
			throw std::exception();
	}
	code = std::atoi(code_str.c_str());

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
