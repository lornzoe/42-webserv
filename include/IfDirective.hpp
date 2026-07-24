/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IfDirective.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 08:30:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/24 08:30:00 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IFDIRECTIVE_HPP
#define IFDIRECTIVE_HPP

#include "BlockDirective.hpp"

class IfDirective : public BlockDirective
{
	private:

	public:
		IfDirective(TokenisedBlock::const_iterator& cit) : BlockDirective(cit) {}
		void print(int) const {}
};

#endif
