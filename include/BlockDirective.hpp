/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BlockDirective.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/11 23:12:29 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/11 23:16:39 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BLOCKDIRECTIVE_HPP
#define BLOCKDIRECTIVE_HPP

#include "Directive.hpp"

class BlockDirective : public Directive
{
	public:
		BlockDirective();
		virtual ~BlockDirective();	
		
		virtual void print(int depth) const = 0;

};
#endif