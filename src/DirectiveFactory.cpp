/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectiveFactory.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 21:00:51 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/22 18:06:42 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "DirectiveFactory.hpp"

using namespace DirectiveFactory;

namespace {
    const std::map<std::string, DirectiveInfo>& getDirectiveMap()
    {
        static std::map<std::string, DirectiveInfo> allowedMap;
        
        if (allowedMap.empty()) {
            // block directives
            allowedMap["server"]               = DirectiveInfo(TYPE_BLOCK, CONTEXT_MAIN);
            allowedMap["location"]             = DirectiveInfo(TYPE_BLOCK, CONTEXT_SERVER);
            allowedMap["if"]                   = DirectiveInfo(TYPE_BLOCK, CONTEXT_SERVER | CONTEXT_LOCATION);

            // simple directives
            allowedMap["listen"]               = DirectiveInfo(TYPE_SIMPLE, CONTEXT_SERVER);
            allowedMap["server_name"]          = DirectiveInfo(TYPE_SIMPLE, CONTEXT_SERVER);
            allowedMap["alias"]                = DirectiveInfo(TYPE_SIMPLE, CONTEXT_LOCATION);

            allowedMap["root"]                 = DirectiveInfo(TYPE_SIMPLE, CONTEXT_SERVER | CONTEXT_LOCATION);
            allowedMap["index"]                = DirectiveInfo(TYPE_SIMPLE, CONTEXT_SERVER | CONTEXT_LOCATION);
            allowedMap["client_max_body_size"] = DirectiveInfo(TYPE_SIMPLE, CONTEXT_SERVER | CONTEXT_LOCATION);
            allowedMap["error_page"]           = DirectiveInfo(TYPE_SIMPLE, CONTEXT_SERVER | CONTEXT_LOCATION);

            allowedMap["return"]               = DirectiveInfo(TYPE_SIMPLE, CONTEXT_SERVER | CONTEXT_LOCATION | CONTEXT_IF);
        }
        return allowedMap;
    }
}

// this exists purely for map to initialise a default value
DirectiveFactory::DirectiveInfo::DirectiveInfo()
{
}

DirectiveFactory::DirectiveInfo::DirectiveInfo(Type t, int i) : type(t), allowedContexts(i) {}

bool DirectiveFactory::canExist(const std::string &directive)
{
    const std::map<std::string, DirectiveInfo>& m = getDirectiveMap();
    return m.find(directive) != m.end();
}

bool DirectiveFactory::isValidInContext(const std::string &directive, Context currentContext)
{
    const std::map<std::string, DirectiveInfo>& m = getDirectiveMap();
    std::map<std::string, DirectiveInfo>::const_iterator it = m.find(directive);
    
    if (it == m.end()) {
        return false;
    }
    
    return (it->second.allowedContexts & currentContext) != 0;
}

Type DirectiveFactory::getType(const std::string &directive)
{
    const std::map<std::string, DirectiveInfo>& m = getDirectiveMap();
    std::map<std::string, DirectiveInfo>::const_iterator it = m.find(directive);
    
    if (it == m.end())
        return DirectiveFactory::TYPE_NONE; // default fallback, value 0
        
    return it->second.type;
}

bool DirectiveFactory::isBlockType(const std::string &directive)
{
    return getType(directive) == DirectiveFactory::TYPE_BLOCK;
}

bool DirectiveFactory::isSimpleType(const std::string &directive)
{
    return getType(directive) == DirectiveFactory::TYPE_SIMPLE;
}
