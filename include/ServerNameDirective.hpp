/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerNameDirective.hpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 08:30:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/27 12:44:49 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERNAMEDIRECTIVE_HPP
#define SERVERNAMEDIRECTIVE_HPP

#include "SimpleDirective.hpp"
#include <string>
#include <vector>

class ServerNameDirective : public SimpleDirective
{
	private:
		std::vector<std::string> names;

	public:
		ServerNameDirective(TokenisedBlock::const_iterator& cit);
		void print(int depth) const;

		const std::vector<std::string>& getNames() const;
};

#endif
