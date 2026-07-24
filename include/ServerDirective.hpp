/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerDirective.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/22 15:37:22 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/22 18:03:31 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERDIRECTIVE_HPP
#define SERVERDIRECTIVE_HPP

#include "BlockDirective.hpp"

class ServerDirective : public BlockDirective
{
	private:
		// typedefs are here to keep the definition within th
		// what does this need again?		
	public:
		ServerDirective(TokenisedBlock::const_iterator& cit);
		~ServerDirective();

		void print(int depth) const;
};

#endif