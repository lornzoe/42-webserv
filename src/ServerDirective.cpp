/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerDirective.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/22 15:44:33 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/01 19:25:19 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerDirective.hpp"
#include "DirectiveFactory.hpp"
#include <exception>
#include <iostream>

ServerDirective::ServerDirective(TokenisedBlock::const_iterator& cit) : BlockDirective(cit)
{
	// validation
	if (args[0] != "server")
		throw std::exception(); // first line necessary elements are not detected
}

ServerDirective::~ServerDirective()
{
}

std::vector<const ListenDirective *> ServerDirective::getListens() const
{
	return getChildren<ListenDirective>();
}

const ServerNameDirective* ServerDirective::getServerName() const
{
	return getChild<ServerNameDirective>();
}

const RootDirective* ServerDirective::getRoot() const
{
	return getChild<RootDirective>();
}

const IndexDirective* ServerDirective::getIndex() const
{
	return getChild<IndexDirective>();
}

std::vector<const ErrorPageDirective *> ServerDirective::getErrorPages() const
{
	return getChildren<ErrorPageDirective>();
}

const ClientMaxBodySizeDirective* ServerDirective::getClientMaxBodySize() const
{
	return getChild<ClientMaxBodySizeDirective>();
}

std::vector<const LocationDirective *> ServerDirective::getLocations() const
{
	return getChildren<LocationDirective>();
}

void ServerDirective::print(int depth) const
{
	for (int i = 0; i < depth; i++)
		std::cout << "  ";
	std::cout << "server {" << std::endl;

	for (std::size_t i = 0; i < directives.size(); i++)
		directives[i]->print(depth + 1);

	for (int i = 0; i < depth; i++)
		std::cout << "  ";
	std::cout << "}" << std::endl;
}
