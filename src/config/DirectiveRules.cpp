/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectiveRules.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 08:04:50 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/29 12:38:37 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "DirectiveRules.hpp"

#include "ServerDirective.hpp"
#include "LocationDirective.hpp"
#include "IfDirective.hpp"
#include "ListenDirective.hpp"
#include "ServerNameDirective.hpp"
#include "AliasDirective.hpp"
#include "RootDirective.hpp"
#include "IndexDirective.hpp"
#include "ClientMaxBodySizeDirective.hpp"
#include "ErrorPageDirective.hpp"
#include "ReturnDirective.hpp"
#include "LimitExceptDirective.hpp"

#include <map>

namespace {

    template <typename T>
    Directive* createDirective(Directive::TokenisedBlock::const_iterator& cit)
    {
        return new T(cit);
    }

    // DirectiveInfo is only used by this file
    // If we need it for other files this struct will be moved to the header
    struct DirectiveInfo {
        DirectiveRules::Type type;
        int allowedContexts;
        DirectiveRules::Creator creator;

        DirectiveInfo() : type(DirectiveRules::TYPE_NONE), allowedContexts(0), creator(0) {}
        DirectiveInfo(DirectiveRules::Type t, int i, DirectiveRules::Creator c) 
            : type(t), allowedContexts(i), creator(c) {}
    };

    const std::map<std::string, DirectiveInfo>& getDirectiveMap()
    {
        static std::map<std::string, DirectiveInfo> allowedMap;

        if (allowedMap.empty()) {
            using namespace DirectiveRules;

            // block directives
            allowedMap["server"]                = DirectiveInfo(TYPE_BLOCK, CONTEXT_MAIN, &createDirective<ServerDirective>);
            allowedMap["location"]              = DirectiveInfo(TYPE_BLOCK, CONTEXT_SERVER, &createDirective<LocationDirective>);
            allowedMap["if"]                    = DirectiveInfo(TYPE_BLOCK, CONTEXT_SERVER | CONTEXT_LOCATION, &createDirective<IfDirective>);

            // simple directives
            allowedMap["listen"]                = DirectiveInfo(TYPE_SIMPLE, CONTEXT_SERVER, &createDirective<ListenDirective>);
            allowedMap["server_name"]           = DirectiveInfo(TYPE_SIMPLE, CONTEXT_SERVER, &createDirective<ServerNameDirective>);
            allowedMap["alias"]                 = DirectiveInfo(TYPE_SIMPLE, CONTEXT_LOCATION, &createDirective<AliasDirective>);

            allowedMap["root"]                  = DirectiveInfo(TYPE_SIMPLE, CONTEXT_SERVER | CONTEXT_LOCATION, &createDirective<RootDirective>);
            allowedMap["index"]                 = DirectiveInfo(TYPE_SIMPLE, CONTEXT_SERVER | CONTEXT_LOCATION, &createDirective<IndexDirective>);
            allowedMap["client_max_body_size"]  = DirectiveInfo(TYPE_SIMPLE, CONTEXT_SERVER | CONTEXT_LOCATION, &createDirective<ClientMaxBodySizeDirective>);
            allowedMap["error_page"]            = DirectiveInfo(TYPE_SIMPLE, CONTEXT_SERVER | CONTEXT_LOCATION, &createDirective<ErrorPageDirective>);

            allowedMap["return"]                = DirectiveInfo(TYPE_SIMPLE, CONTEXT_SERVER | CONTEXT_LOCATION | CONTEXT_IF, &createDirective<ReturnDirective>);
            allowedMap["limit_except"]          = DirectiveInfo(TYPE_SIMPLE, CONTEXT_LOCATION, &createDirective<LimitExceptDirective>);
        }
        return allowedMap;
    }

} // anonymous namespace

namespace DirectiveRules {

    bool canExist(const std::string &directive)
    {
        const std::map<std::string, DirectiveInfo>& m = getDirectiveMap();
        return m.find(directive) != m.end();
    }

    bool isValidInContext(const std::string &directive, Context currentContext)
    {
        const std::map<std::string, DirectiveInfo>& m = getDirectiveMap();
        std::map<std::string, DirectiveInfo>::const_iterator it = m.find(directive);

        if (it == m.end()) {
            return false;
        }

        return (it->second.allowedContexts & currentContext) != 0;
    }

    Type getType(const std::string &directive)
    {
        const std::map<std::string, DirectiveInfo>& m = getDirectiveMap();
        std::map<std::string, DirectiveInfo>::const_iterator it = m.find(directive);

        if (it == m.end())
            return TYPE_NONE;

        return it->second.type;
    }

    bool isBlockType(const std::string &directive)
    {
        return getType(directive) == TYPE_BLOCK;
    }

    bool isSimpleType(const std::string &directive)
    {
        return getType(directive) == TYPE_SIMPLE;
    }

    Creator getCreator(const std::string &directive)
    {
        const std::map<std::string, DirectiveInfo>& m = getDirectiveMap();
        std::map<std::string, DirectiveInfo>::const_iterator it = m.find(directive);

        if (it == m.end())
            return 0;

        return it->second.creator;
    }

}
