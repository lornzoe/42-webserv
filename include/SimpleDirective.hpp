/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SimpleDirective.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/11 22:13:59 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/13 17:11:51 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIMPLEDIRECTIVE_HPP
#define SIMPLEDIRECTIVE_HPP

#include "Directive.hpp"
#include <string>
#include <vector>

class SimpleDirective : public Directive 
{
	protected:
		std::vector<std::string> args; // debug
	public:
		SimpleDirective();
		SimpleDirective(char *raw_directive);
		SimpleDirective(std::string raw_directive);
		virtual ~SimpleDirective();

		virtual void print(int depth) const = 0;
};

#endif