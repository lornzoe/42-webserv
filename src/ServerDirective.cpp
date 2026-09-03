/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerDirective.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/22 15:44:33 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/21 19:53:30 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerDirective.hpp"
#include "DirectiveFactory.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <algorithm>

namespace {
	const char* WILDCARD_HOST = "0.0.0.0";

	const LocationDirective* matchLocation(
		const std::vector<const LocationDirective *>& locations,
		const std::string& uri)
	{
		const LocationDirective* best = NULL;
		std::size_t bestLen = 0;

		for (std::size_t i = 0; i < locations.size(); i++)
		{
			const std::string& prefix = locations[i]->getPath();
			if (prefix.empty() || prefix.size() > uri.size())
				continue;
			if (uri.compare(0, prefix.size(), prefix) != 0)
				continue;
			if (prefix.size() > bestLen)
			{
				best = locations[i];
				bestLen = prefix.size();
			}
		}
		return best;
	}

	bool statRegularFile(const std::string& path, struct stat& st)
	{
		return stat(path.c_str(), &st) != -1 && S_ISREG(st.st_mode);
	}

	const std::string* findCode(const std::vector<const ErrorPageDirective *>& pages, int code)
    {
        const std::string* match = NULL;
        for (std::size_t i = 0; i < pages.size(); i++)
        {
            const std::vector<int>& codes = pages[i]->getCodes();
            if (std::find(codes.begin(), codes.end(), code) != codes.end())
                match = &pages[i]->getPath();
        }
        return match;
    }
}

ServerDirective::ServerDirective(TokenisedBlock::const_iterator& cit) : BlockDirective(cit)
{
	// validation
	if (args[0] != "server")
		throw std::runtime_error("server: malformed server block (first token was not 'server')");

	requireAtLeastOne<ListenDirective>("server", "listen");

	requireAtMostOne<RootDirective>("server", "root");
	requireAtMostOne<IndexDirective>("server", "index");
	requireAtMostOne<ClientMaxBodySizeDirective>("server", "client_max_body_size");
	requireAtMostOne<ReturnDirective>("server", "return");
	requireAtMostOne<ServerNameDirective>("server", "server_name");
}

ServerDirective::~ServerDirective()
{
}

std::vector<const ListenDirective *> ServerDirective::getListens() const
{
	return getChildren<ListenDirective>();
}

bool ServerDirective::isMatch(const std::string& host, const std::string& port) const
{
	if (host.empty() || port.empty())
		return false;

	std::vector<const ListenDirective *> listens = getListens();
	for (std::size_t i = 0; i < listens.size(); i++)
	{
		std::ostringstream listen_port;
		listen_port << listens[i]->getPort();
		if (listen_port.str() != port)
			continue;

		const std::string& bind = listens[i]->getHost();
		if (bind == WILDCARD_HOST || bind == host)
			return true;
	}
	return false;
}

bool ServerDirective::isMatch(const std::string& host, int port) const
{
	std::ostringstream port_str;
	port_str << port;
	return isMatch(host, port_str.str());
}

const ServerNameDirective* ServerDirective::getServerName() const
{
	return getChild<ServerNameDirective>();
}

const RootDirective* ServerDirective::getRoot() const
{
	return getChild<RootDirective>();
}

const IndexDirective* ServerDirective::getIndex() const
{
	return getChild<IndexDirective>();
}

std::vector<const ErrorPageDirective *> ServerDirective::getErrorPages() const
{
	return getChildren<ErrorPageDirective>();
}

const ClientMaxBodySizeDirective* ServerDirective::getClientMaxBodySize() const
{
	return getChild<ClientMaxBodySizeDirective>();
}

std::vector<const LocationDirective *> ServerDirective::getLocations() const
{
	return getChildren<LocationDirective>();
}

ServerDirective::ResourcePath ServerDirective::getResource(const std::string& uri) const
{
	ResourcePath none = std::make_pair(false, std::string());

	std::vector<const LocationDirective *> locations = getLocations();
	const LocationDirective* loc = matchLocation(locations, uri);

	std::string subpath = loc ? uri.substr(loc->getPath().size()) : uri;

	std::string base;
	if (loc && loc->getAlias())
		base = loc->getAlias()->getPath();
	else if (loc && loc->getRoot())
		base = loc->getRoot()->getPath();
	else if (getRoot())
		base = getRoot()->getPath();
	else
		return none;

	std::string fsPath = base + subpath;

	struct stat st;
	if (stat(fsPath.c_str(), &st) == -1)
		return none;

	if (S_ISREG(st.st_mode))
		return std::make_pair(true, fsPath);

	if (S_ISDIR(st.st_mode))
	{
		const IndexDirective* index = (loc && loc->getIndex()) ? loc->getIndex() : getIndex();
		if (index)
		{
			const std::vector<std::string>& files = index->getFiles();
			for (std::size_t i = 0; i < files.size(); i++)
			{
				std::string path = fsPath;
				if (!path.empty() && path[path.size() - 1] != '/')
					path += "/";
				path += files[i];

				struct stat cst;
				if (statRegularFile(path, cst))
					return std::make_pair(true, path);
			}
		}
		return none;
	}

	return none;
}

ServerDirective::ResourcePath ServerDirective::getErrorPage(const std::string &uri, int code) const
{
    const LocationDirective* loc = matchLocation(getLocations(), uri);

    // location wins if it defines this code
    if (loc)
    {
        const std::string* p = findCode(loc->getErrorPages(), code);
        if (p)
            return std::make_pair(true, *p);
    }
    // fall back to server-level
    const std::string* p = findCode(getErrorPages(), code);
    if (p)
        return std::make_pair(true, *p);
    return std::make_pair(false, std::string());
}

void ServerDirective::print(int depth) const
{
	for (int i = 0; i < depth; i++)
		std::cout << "  ";
	std::cout << "server {" << std::endl;

	for (std::size_t i = 0; i < directives.size(); i++)
		directives[i]->print(depth + 1);

	for (int i = 0; i < depth; i++)
		std::cout << "  ";
	std::cout << "}" << std::endl;
}
