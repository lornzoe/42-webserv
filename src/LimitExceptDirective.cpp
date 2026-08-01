/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LimitExceptDirective.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/29 12:22:58 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/01 19:27:41 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LimitExceptDirective.hpp"
#include <exception>
#include <iostream>

LimitExceptDirective::LimitExceptDirective(TokenisedBlock::const_iterator &cit) : SimpleDirective(cit)
{
	if (args.size() > 4)
		throw std::exception(); // limit_except can take up to 3 args
	for (std::size_t i = 1; i < args.size(); i++)
		methods.push_back(args[i]);
}

const std::vector<std::string>& LimitExceptDirective::getMethods() const
{
	return methods;
}

void LimitExceptDirective::print(int depth) const
{
	for (int i = 0; i < depth; i++)
		std::cout << "  ";
	std::cout << "limit_except ";
	for (std::size_t i = 1; i < args.size(); i++)
	{
		std::cout << args[i];
		if (i + 1 != args.size())
			std::cout << " ";
	}
	std::cout << std::endl;
}
