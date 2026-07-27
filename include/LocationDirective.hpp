/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationDirective.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 08:30:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/27 12:44:56 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONDIRECTIVE_HPP
#define LOCATIONDIRECTIVE_HPP

#include "BlockDirective.hpp"
#include <iostream>

class LocationDirective : public BlockDirective
{
	private:

	public:
		LocationDirective(TokenisedBlock::const_iterator& cit) : BlockDirective(cit) {}

		void print(int depth) const
		{
			for (int i = 0; i < depth; i++)
				std::cout << "  ";
			std::cout << "location";
			for (std::size_t i = 1; i < args.size(); i++)
				std::cout << " " << args[i];
			std::cout << " {" << std::endl;

			for (std::size_t i = 0; i < directives.size(); i++)
				directives[i]->print(depth + 1);

			for (int i = 0; i < depth; i++)
				std::cout << "  ";
			std::cout << "}" << std::endl;
		}
};

#endif
