/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BlockDirective.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/11 23:12:29 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/13 17:10:49 by lyanga           ###   ########.fr       */
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
		std::vector<std::string> args; // BlockDirective args, if any
		std::list<Directive> directives; // list of directives in block
	public:
		BlockDirective();
		BlockDirective(std::string raw_block);
		virtual ~BlockDirective();	
		
		virtual void print(int depth) const = 0;
};
#endif