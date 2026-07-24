/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientMaxBodySizeDirective.hpp                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 08:30:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/24 08:30:00 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENTMAXBODYSIZEDIRECTIVE_HPP
#define CLIENTMAXBODYSIZEDIRECTIVE_HPP

#include "SimpleDirective.hpp"

class ClientMaxBodySizeDirective : public SimpleDirective
{
	private:

	public:
		ClientMaxBodySizeDirective(TokenisedBlock::const_iterator& cit) : SimpleDirective(cit) {}
		void print(int) const {}
};

#endif
