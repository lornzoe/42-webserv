/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BlockDirective.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/22 18:05:35 by lyanga            #+#    #+#             */
/*   Updated: 2026/11/16 17:19:16 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "BlockDirective.hpp"
#include "DirectiveRules.hpp"
#include "DirectiveFactory.hpp"
#include <stdexcept>

BlockDirective::BlockDirective(TokenisedBlock::const_iterator &cit) : Directive(*cit)
{
	cit++; // move past this block's own opening line

	// nested blocks fully consume their own contents (including their
	// closing brace) recursively, so the only "}" this loop ever sees
	// directly is this block's own.
	try {
		while (cit->front() != "}")
		{
			if (DirectiveRules::isSimpleType(cit->front()) || DirectiveRules::isBlockType(cit->front()))
				directives.push_back(DirectiveFactory::createDirective(cit));
			else
				throw std::runtime_error("unknown directive '" + cit->front() + "'");
		}
		cit++; // consume this block's own closing "}"
	}
	catch (std::runtime_error &e)
	{
		for (std::size_t i = 0; i < directives.size(); i++)
			delete directives[i];
		throw;
	}
}

bool BlockDirective::addDirective(Directive *directive)
{
	if (!directive)
		return false;
	directives.push_back(directive);
	return true;
}

const std::vector<Directive *> &BlockDirective::getChildren() const
{
	return directives;
}

BlockDirective::~BlockDirective()
{
	for (std::size_t i = 0; i < directives.size(); i++)
	{
		delete directives[i];
	}
}