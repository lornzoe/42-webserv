/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ReturnDirective.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 08:30:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/27 12:44:55 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RETURNDIRECTIVE_HPP
#define RETURNDIRECTIVE_HPP

#include "SimpleDirective.hpp"
#include <string>

class ReturnDirective : public SimpleDirective
{
	private:
		int code;
		std::string body;

	public:
		ReturnDirective(TokenisedBlock::const_iterator& cit);
		void print(int depth) const;

		int getCode() const;
		const std::string& getBody() const;
};

#endif
