/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerDirective.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/22 15:44:33 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/22 18:06:58 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerDirective.hpp"
#include "DirectiveFactory.hpp"
#include <exception>

namespace
{
	
}

ServerDirective::ServerDirective(TokenisedBlock::const_iterator& cit) : BlockDirective(cit)
{
	// validation
	if (args[0] != "server")
		throw std::exception(); // first line necessary elements are not detected
	
	// initialise the arguments if any.

}

ServerDirective::~ServerDirective()
{
}

void ServerDirective::print(int depth) const
{
	(void)depth;
}
