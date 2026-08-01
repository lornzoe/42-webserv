/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerDirective.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/22 15:37:22 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/01 19:25:11 by lyanga           ###   ########.fr       */
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

class ServerDirective : public BlockDirective
{
	public:
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
};

#endif