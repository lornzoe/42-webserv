/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerDirective.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/22 15:37:22 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/02 04:14:10 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERDIRECTIVE_HPP
#define SERVERDIRECTIVE_HPP

#include "BlockDirective.hpp"
#include "ListenDirective.hpp"
#include "ServerNameDirective.hpp"
#include "RootDirective.hpp"
#include "IndexDirective.hpp"
#include "ErrorPageDirective.hpp"
#include "ClientMaxBodySizeDirective.hpp"
#include "LocationDirective.hpp"
#include <vector>
#include <string>
#include <utility>

class ServerDirective : public BlockDirective
{
	public:
		// first: true if path exists and is a regular file.
		// second: resolved filesystem path; empty if unresolved.
		typedef std::pair<bool, std::string> ResourcePath;

		ServerDirective(TokenisedBlock::const_iterator& cit);
		~ServerDirective();

		void print(int depth) const;

		std::vector<const ListenDirective *> getListens() const;
		const ServerNameDirective* getServerName() const;
		const RootDirective* getRoot() const;
		const IndexDirective* getIndex() const;
		std::vector<const ErrorPageDirective *> getErrorPages() const;
		const ClientMaxBodySizeDirective* getClientMaxBodySize() const;
		std::vector<const LocationDirective *> getLocations() const;

		ResourcePath getResource(const std::string& uri) const;
};

#endif