/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LimitExceptDirective.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/29 12:22:58 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/29 12:42:33 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LimitExceptDirective.hpp"
#include <exception>
#include <iostream>

LimitExceptDirective::LimitExceptDirective(TokenisedBlock::const_iterator &cit) : SimpleDirective(cit)
{
	if (args.size() > 4)
		throw std::exception(); // limit_except can take up to 3 args
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
