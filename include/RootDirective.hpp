/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RootDirective.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 08:30:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/24 08:30:00 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROOTDIRECTIVE_HPP
#define ROOTDIRECTIVE_HPP

#include "SimpleDirective.hpp"

class RootDirective : public SimpleDirective
{
	private:

	public:
		RootDirective(TokenisedBlock::const_iterator& cit) : SimpleDirective(cit) {}
		void print(int) const {}
};

#endif
