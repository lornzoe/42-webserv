/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AllowedDirectives.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 21:00:51 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/14 17:12:40 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "AllowedDirectives.hpp"

static const std::map<std::string, int>& getDirectiveMap()
{
    static std::map<std::string, int> allowedMap;
    
    if (allowedMap.empty()) {
        allowedMap["server"]               = AllowedDirectives::CONTEXT_MAIN;

        allowedMap["listen"]               = AllowedDirectives::CONTEXT_SERVER;
        allowedMap["server_name"]          = AllowedDirectives::CONTEXT_SERVER;
        allowedMap["location"]             = AllowedDirectives::CONTEXT_SERVER;

        allowedMap["alias"]                = AllowedDirectives::CONTEXT_LOCATION;

        // multi 
        int serverAndLocation = AllowedDirectives::CONTEXT_SERVER 
                                | AllowedDirectives::CONTEXT_LOCATION;
        allowedMap["root"]                 = serverAndLocation;
        allowedMap["index"]                = serverAndLocation;
        allowedMap["client_max_body_size"] = serverAndLocation;
        allowedMap["error_page"]           = serverAndLocation;
        allowedMap["return"]               = serverAndLocation;
        allowedMap["if"]                   = serverAndLocation;

        // if blocks 
        allowedMap["return"]              |= AllowedDirectives::CONTEXT_IF;
    }
    return allowedMap;
}

bool AllowedDirectives::canExist(const std::string &directive)
{
    const std::map<std::string, int>& m = getDirectiveMap();
    return m.find(directive) != m.end();
}

bool AllowedDirectives::isValidInContext(const std::string &directive, Context currentContext)
{
    const std::map<std::string, int>& m = getDirectiveMap();
    std::map<std::string, int>::const_iterator it = m.find(directive);
    
    if (it == m.end()) {
        return false;
    }
    
    return (it->second & currentContext) != 0;
}