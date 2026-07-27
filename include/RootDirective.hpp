/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RootDirective.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 14:03:55 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/27 14:03:58 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROOTDIRECTIVE_HPP
#define ROOTDIRECTIVE_HPP

#include "SimpleDirective.hpp"
#include <string>

class RootDirective : public SimpleDirective
{
	private:
		std::string path;

	public:
		RootDirective(TokenisedBlock::const_iterator& cit);
		void print(int depth) const;

		const std::string& getPath() const;
};

#endif
