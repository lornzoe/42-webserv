/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorPageDirective.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 08:30:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/24 08:30:00 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ERRORPAGEDIRECTIVE_HPP
#define ERRORPAGEDIRECTIVE_HPP

#include "SimpleDirective.hpp"

class ErrorPageDirective : public SimpleDirective
{
	private:

	public:
		ErrorPageDirective(TokenisedBlock::const_iterator& cit) : SimpleDirective(cit) {}
		void print(int) const {}
};

#endif
