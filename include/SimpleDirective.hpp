/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SimpleDirective.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/11 22:13:59 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/27 12:44:48 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIMPLEDIRECTIVE_HPP
#define SIMPLEDIRECTIVE_HPP

#include "Directive.hpp"
#include <string>
#include <vector>

class SimpleDirective : public Directive
{
	public:
		SimpleDirective(TokenisedBlock::const_iterator& cit);
		virtual ~SimpleDirective();

		virtual void print(int depth) const = 0;
};

#endif