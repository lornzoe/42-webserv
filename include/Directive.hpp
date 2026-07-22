/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Directive.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/11 21:06:29 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/22 17:17:46 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DIRECTIVE_HPP
#define DIRECTIVE_HPP

#include <vector>
#include <string>

class Directive
{
	protected:
		typedef const std::vector<std::string> TokenisedLine;
		std::vector<std::string> args;
	public:
		Directive(TokenisedLine& raw_directive);
		
		virtual ~Directive();
		virtual void print(int depth) const = 0;
};

#endif
