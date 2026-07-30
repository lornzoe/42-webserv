/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectiveFactory.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 21:00:23 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/24 08:38:44 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DIRECTIVEFACTORY_HPP
#define DIRECTIVEFACTORY_HPP

#include "Directive.hpp"

// Responsible for constructing Directive objects from tokenised input.
// The grammar/validation rules now live in DirectiveRules.
class DirectiveFactory {
    public:
        static Directive *createDirective(Directive::TokenisedBlock::const_iterator& cit);
    private:
        DirectiveFactory(); // never defined, class is not instantiable
};

#endif
