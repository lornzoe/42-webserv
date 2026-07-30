/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorPageDirective.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 08:30:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/27 12:45:08 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ERRORPAGEDIRECTIVE_HPP
#define ERRORPAGEDIRECTIVE_HPP

#include "SimpleDirective.hpp"
#include <string>
#include <vector>

class ErrorPageDirective : public SimpleDirective
{
	private:
		std::vector<int> codes;
		std::string path;

	public:
		ErrorPageDirective(TokenisedBlock::const_iterator& cit);
		void print(int depth) const;

		const std::vector<int>& getCodes() const;
		const std::string& getPath() const;
};

#endif
