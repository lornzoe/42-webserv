/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectiveRules.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 08:04:50 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/24 08:13:18 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DIRECTIVERULES_HPP
#define DIRECTIVERULES_HPP

#include <map>
#include <string>

// Stateless registry describing the config grammar: which directives exist,
// what shape they take (simple vs block) and which contexts they may appear in.
// Not instantiable, everything is a static query against a single shared table.
class DirectiveRules {
public:
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

    static bool canExist(const std::string& directive);
    static bool isValidInContext(const std::string& directive, Context currentContext);

    static Type getType(const std::string& directive);
    static bool isBlockType(const std::string& directive);
    static bool isSimpleType(const std::string& directive);

private:
    struct DirectiveInfo {
        Type type;
        int allowedContexts;
        DirectiveInfo();
        DirectiveInfo(Type, int);
    };

    static const std::map<std::string, DirectiveInfo>& getDirectiveMap();

    DirectiveRules(); // never defined, class is not instantiable
};

#endif
