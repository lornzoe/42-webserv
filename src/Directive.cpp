/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Directive.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/11 22:01:16 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/22 17:19:02 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Directive.hpp"

Directive::Directive(TokenisedLine& raw_directive)
{
	// start at first element, till second last element.
	// follow structure similar to argv:
	// 		first element is the directive name,
	// 		onwards are the args
	for (TokenisedLine::const_iterator cit = raw_directive.begin(); cit != raw_directive.end() - 1; ++cit)
	{
		args.push_back(*cit);
	}
}

Directive::~Directive() {}
