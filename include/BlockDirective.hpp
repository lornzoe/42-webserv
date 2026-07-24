/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BlockDirective.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/11 23:12:29 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/22 17:14:42 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BLOCKDIRECTIVE_HPP
#define BLOCKDIRECTIVE_HPP

#include "Directive.hpp"
#include <string>
#include <vector>
#include <list>

class BlockDirective : public Directive
{
	protected:
		std::vector<Directive *> directives; // list of directives in block
	public:
		BlockDirective(TokenisedBlock::const_iterator& cit);
		virtual ~BlockDirective();	
		
		virtual void print(int depth) const = 0;
};
#endif