/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RootDirective.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 00:00:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/09/13 11:07:07 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RootDirective.hpp"
#include <stdexcept>
#include <iostream>

RootDirective::RootDirective(TokenisedBlock::const_iterator& cit) : SimpleDirective(cit)
{
	if (args.size() != 2)
		throw std::runtime_error("root: expects exactly one argument (e.g. 'root /var/www;')");

	path = args[1];
}

void RootDirective::print(int depth) const
{
	for (int i = 0; i < depth; i++)
		std::cout << "  ";
	std::cout << "root " << path << ";" << std::endl;
}

const std::string& RootDirective::getPath() const { return path; }
