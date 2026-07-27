/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ListenDirective.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/22 16:36:09 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/27 12:45:00 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LISTENDIRECTIVE_HPP
#define LISTENDIRECTIVE_HPP

#include "SimpleDirective.hpp"
#include <string>

class ListenDirective : public SimpleDirective
{
	private:
		std::string host;
		int port;

	public:
		ListenDirective(TokenisedBlock::const_iterator& cit);
		void print(int depth) const;

		const std::string& getHost() const;
		int getPort() const;
};

#endif
