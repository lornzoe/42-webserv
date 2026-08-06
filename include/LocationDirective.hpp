/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationDirective.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 08:30:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/01 19:30:53 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONDIRECTIVE_HPP
#define LOCATIONDIRECTIVE_HPP

#include "BlockDirective.hpp"
#include "RootDirective.hpp"
#include "AliasDirective.hpp"
#include "IndexDirective.hpp"
#include "ErrorPageDirective.hpp"
#include "ClientMaxBodySizeDirective.hpp"
#include "ReturnDirective.hpp"
#include "LimitExceptDirective.hpp"
#include <vector>

class LocationDirective : public BlockDirective
{
	private:

	public:
		LocationDirective(TokenisedBlock::const_iterator& cit);

		void print(int depth) const;

		const std::string& getPath() const;
		const RootDirective* getRoot() const;
		const AliasDirective* getAlias() const;
		const IndexDirective* getIndex() const;
		const ClientMaxBodySizeDirective* getClientMaxBodySize() const;
		const ReturnDirective* getReturn() const;
		const LimitExceptDirective* getLimitExcept() const;
		std::vector<const ErrorPageDirective *> getErrorPages() const;
		std::vector<const LocationDirective *> getLocations() const;
};

#endif
