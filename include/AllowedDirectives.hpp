/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AllowedDirectives.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 21:00:23 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/14 19:14:41 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ALLOWEDDIRECTIVES_HPP
#define ALLOWEDDIRECTIVES_HPP

#include <map>
#include <string>

namespace AllowedDirectives {
    struct DirectiveInfo {
        Type type;
        int allowedContexts;
        DirectiveInfo(Type, int);
    };

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
    
    bool canExist(const std::string& directive);
    bool isValidInContext(const std::string& directive, Context currentContext);

    Type getType(const std::string& directive);
    bool isBlockType(const std::string& directive);
    bool isSimpleType(const std::string& directive);

}

#endif
