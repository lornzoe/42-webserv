/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationDirective.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 08:30:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/24 08:30:00 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONDIRECTIVE_HPP
#define LOCATIONDIRECTIVE_HPP

#include "BlockDirective.hpp"

class LocationDirective : public BlockDirective
{
	private:

	public:
		LocationDirective(TokenisedBlock::const_iterator& cit) : BlockDirective(cit) {}
		void print(int) const {}
};

#endif
