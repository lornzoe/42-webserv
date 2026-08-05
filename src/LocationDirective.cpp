/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationDirective.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/01 19:11:46 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/06 02:20:32 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationDirective.hpp"
#include <stdexcept>
#include <iostream>

LocationDirective::LocationDirective(TokenisedBlock::const_iterator& cit) : BlockDirective(cit)
{
	// validation
	if (args[0] != "location")
		throw std::runtime_error("location: malformed location block (first token was not 'location')");
	if (args.size() != 2)
		throw std::runtime_error("location: expects exactly one path argument (e.g. 'location /uploads {')");

	requireAtMostOne<RootDirective>("location", "root");
	requireAtMostOne<AliasDirective>("location", "alias");
	requireAtMostOne<IndexDirective>("location", "index");
	requireAtMostOne<ClientMaxBodySizeDirective>("location", "client_max_body_size");
	requireAtMostOne<ReturnDirective>("location", "return");
	requireAtMostOne<LimitExceptDirective>("location", "limit_except");

	// even though getResource() prefers alias,
	// throw an error so that we won't have to deal with this ambiguity.
	if (getRoot() && getAlias())
		throw std::runtime_error("location: 'root' and 'alias' cannot both be set in the same block");
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
	// the constructor guarantees args[1] exists
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
