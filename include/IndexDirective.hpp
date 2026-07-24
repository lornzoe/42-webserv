/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IndexDirective.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 08:30:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/24 08:30:00 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef INDEXDIRECTIVE_HPP
#define INDEXDIRECTIVE_HPP

#include "SimpleDirective.hpp"

class IndexDirective : public SimpleDirective
{
	private:

	public:
		IndexDirective(TokenisedBlock::const_iterator& cit) : SimpleDirective(cit) {}
		void print(int) const {}
};

#endif
