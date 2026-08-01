/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectiveFactory.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 21:00:51 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/02 07:18:02 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "DirectiveFactory.hpp"
#include "DirectiveRules.hpp"
#include <stdexcept>

DirectiveFactory::DirectiveFactory() {}

Directive* DirectiveFactory::createDirective(Directive::TokenisedBlock::const_iterator& cit)
{
    DirectiveRules::Creator creator = DirectiveRules::getCreator(cit->front());

    if (!creator)
        throw std::runtime_error("no directive creator registered for '" + cit->front() + "'");

    return creator(cit);
}