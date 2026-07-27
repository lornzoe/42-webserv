/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientMaxBodySizeDirective.hpp                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 08:30:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/27 12:45:11 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENTMAXBODYSIZEDIRECTIVE_HPP
#define CLIENTMAXBODYSIZEDIRECTIVE_HPP

#include "SimpleDirective.hpp"
#include <cstddef>

class ClientMaxBodySizeDirective : public SimpleDirective
{
	private:
		std::size_t bytes;

	public:
		ClientMaxBodySizeDirective(TokenisedBlock::const_iterator& cit);
		void print(int depth) const;

		std::size_t getBytes() const;
};

#endif
