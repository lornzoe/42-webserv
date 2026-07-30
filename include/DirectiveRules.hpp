/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectiveRules.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 08:04:50 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/28 12:56:18 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DIRECTIVERULES_HPP
#define DIRECTIVERULES_HPP

#include "Directive.hpp"
#include <string>

namespace DirectiveRules {

    enum Type {
        TYPE_NONE = 0,
        TYPE_SIMPLE,
        TYPE_BLOCK
    };

    enum Context {
        CONTEXT_MAIN     = 1 << 0, // root
        CONTEXT_SERVER   = 1 << 1, // inside a server {} block
        CONTEXT_LOCATION = 1 << 2, // inside a location {} block
        CONTEXT_IF       = 1 << 3  // inside an if {} block
    };

    // constructs the concrete Directive subclass for a given directive name
    typedef Directive* (*Creator)(Directive::TokenisedBlock::const_iterator&);

    bool canExist(const std::string& directive);
    bool isValidInContext(const std::string& directive, Context currentContext);

    Type getType(const std::string& directive);
    bool isBlockType(const std::string& directive);
    bool isSimpleType(const std::string& directive);

    Creator getCreator(const std::string& directive);

}

#endif
