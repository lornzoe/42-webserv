/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IndexDirective.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 08:30:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/27 12:45:06 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef INDEXDIRECTIVE_HPP
#define INDEXDIRECTIVE_HPP

#include "SimpleDirective.hpp"
#include <string>
#include <vector>

class IndexDirective : public SimpleDirective
{
	private:
		std::vector<std::string> files;

	public:
		IndexDirective(TokenisedBlock::const_iterator& cit);
		void print(int depth) const;

		const std::vector<std::string>& getFiles() const;
};

#endif
