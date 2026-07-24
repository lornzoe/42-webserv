/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectiveFactory.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 21:00:51 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/24 09:29:14 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "DirectiveFactory.hpp"
#include "DirectiveRules.hpp"
#include <exception>

DirectiveFactory::DirectiveFactory() {}

Directive* DirectiveFactory::createDirective(Directive::TokenisedBlock::const_iterator& cit)
{
    DirectiveRules::Creator creator = DirectiveRules::getCreator(cit->front());

    if (!creator)
        throw std::exception(); // unhandled directive, should not happen after validation

    return creator(cit);
}