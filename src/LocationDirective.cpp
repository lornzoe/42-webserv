/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationDirective.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/01 19:11:46 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/02 04:13:13 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationDirective.hpp"
#include <iostream>

LocationDirective::LocationDirective(TokenisedBlock::const_iterator& cit) : BlockDirective(cit)
{
}

void LocationDirective::print(int depth) const
{
	for (int i = 0; i < depth; i++)
		std::cout << "  ";
	std::cout << "location";
	for (std::size_t i = 1; i < args.size(); i++)
		std::cout << " " << args[i];
	std::cout << " {" << std::endl;

	for (std::size_t i = 0; i < directives.size(); i++)
		directives[i]->print(depth + 1);

	for (int i = 0; i < depth; i++)
		std::cout << "  ";
	std::cout << "}" << std::endl;
}

const std::string& LocationDirective::getPath() const
{
	static const std::string empty;
	if (args.size() < 2)
		return empty;
	return args[1];
}

const RootDirective* LocationDirective::getRoot() const
{
	return getChild<RootDirective>();
}

const AliasDirective* LocationDirective::getAlias() const
{
	return getChild<AliasDirective>();
}

const IndexDirective* LocationDirective::getIndex() const
{
	return getChild<IndexDirective>();
}

const ClientMaxBodySizeDirective* LocationDirective::getClientMaxBodySize() const
{
	return getChild<ClientMaxBodySizeDirective>();
}

const ReturnDirective* LocationDirective::getReturn() const
{
	return getChild<ReturnDirective>();
}

const LimitExceptDirective* LocationDirective::getLimitExcept() const
{
	return getChild<LimitExceptDirective>();
}

std::vector<const ErrorPageDirective *> LocationDirective::getErrorPages() const
{
	return getChildren<ErrorPageDirective>();
}

std::vector<const LocationDirective *> LocationDirective::getLocations() const
{
	return getChildren<LocationDirective>();
}
