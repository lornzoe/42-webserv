/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AliasDirective.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 00:00:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/02 07:16:59 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "AliasDirective.hpp"
#include <stdexcept>
#include <iostream>

AliasDirective::AliasDirective(TokenisedBlock::const_iterator& cit) : SimpleDirective(cit)
{
	if (args.size() != 2)
		throw std::runtime_error("alias: expects exactly one argument (e.g. 'alias /var/www/html;')");

	path = args[1];
}

void AliasDirective::print(int depth) const
{
	for (int i = 0; i < depth; i++)
		std::cout << "  ";
	std::cout << "alias " << path << ";" << std::endl;
}

const std::string& AliasDirective::getPath() const { return path; }
