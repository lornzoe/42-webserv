/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AliasDirective.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 08:30:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/24 08:30:42 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ALIASDIRECTIVE_HPP
#define ALIASDIRECTIVE_HPP

#include "SimpleDirective.hpp"

class AliasDirective : public SimpleDirective
{
	private:

	public:
		AliasDirective(TokenisedBlock::const_iterator& cit) : SimpleDirective(cit) {}
		void print(int) const {}
};

#endif
