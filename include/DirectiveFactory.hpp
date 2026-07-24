/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectiveFactory.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 21:00:23 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/24 08:22:04 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DIRECTIVEFACTORY_HPP
#define DIRECTIVEFACTORY_HPP

#include "Directive.h"

// Responsible for constructing Directive objects from tokenised input.
// The grammar/validation rules now live in DirectiveRules.
class DirectiveFactory {
    public:
    private:
        DirectiveFactory(); // never defined, class is not instantiable
};

#endif
