/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerNameDirective.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 00:00:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/02 07:18:32 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerNameDirective.hpp"
#include <stdexcept>
#include <iostream>

ServerNameDirective::ServerNameDirective(TokenisedBlock::const_iterator& cit) : SimpleDirective(cit)
{
	if (args.size() < 2)
		throw std::runtime_error("server_name: requires at least one name argument");

	for (std::size_t i = 1; i < args.size(); i++)
		names.push_back(args[i]);
}

void ServerNameDirective::print(int depth) const
{
	for (int i = 0; i < depth; i++)
		std::cout << "  ";
	std::cout << "server_name";
	for (std::size_t i = 0; i < names.size(); i++)
		std::cout << " " << names[i];
	std::cout << ";" << std::endl;
}

const std::vector<std::string>& ServerNameDirective::getNames() const { return names; }
