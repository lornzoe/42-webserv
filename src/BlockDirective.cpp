/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BlockDirective.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/22 18:05:35 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/22 18:22:16 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "BlockDirective.hpp"
#include "DirectiveFactory.hpp"

BlockDirective::BlockDirective(TokenisedBlock::const_iterator &cit) : Directive(*cit)
{
	std::size_t braces = 1;
	cit++;

	// start cit from the first line in the contents
	while (braces != 0)
	{
		if (cit->front() == "}")
		{
			braces--;
			continue;
		}
		if (DirectiveFactory::isSimpleType(cit->front()))
		{
			// handle simple type
		}
		else if (DirectiveFactory::isBlockType(cit->front()))
		{
			// handle block type
		}
		else
			throw std::exception(); // not a valid directive
		cit++;
	}
}

BlockDirective::~BlockDirective()
{
	for (std::size_t i = 0; i < directives.size(); i++)
	{
		delete directives[i];
	}
}