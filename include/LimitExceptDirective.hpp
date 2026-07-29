/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LimitExceptDirective.hpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/29 12:00:44 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/29 12:22:14 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LIMITEXCEPTDIRECTIVE_HPP
#define LIMITEXCEPTDIRECTIVE_HPP

#include "SimpleDirective.hpp"

// Our implementation of LimitExceptDirective will only be a simple directive.
// We only plan to support the 3 mandatory HTTP methods (GET, POST, DELETE)

// Both unlike the NGINX version of it, until circumstances change...

// Usage: limit_except [HTTP_METHODS]; 
// e.g. limit_except GET;
//		limit_except GET POST DELETE;

class LimitExceptDirective : public SimpleDirective
{
	private:
		std::vector<std::string> methods;

	public:
		LimitExceptDirective(TokenisedBlock::const_iterator& cit);
		void print(int depth) const;
};

#endif