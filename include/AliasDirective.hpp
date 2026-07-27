/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AliasDirective.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 08:30:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/27 12:45:12 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ALIASDIRECTIVE_HPP
#define ALIASDIRECTIVE_HPP

#include "SimpleDirective.hpp"
#include <string>

class AliasDirective : public SimpleDirective
{
	private:
		std::string path;

	public:
		AliasDirective(TokenisedBlock::const_iterator& cit);
		void print(int depth) const;

		const std::string& getPath() const;
};

#endif
