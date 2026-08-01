/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RootDirective.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 00:00:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/02 03:35:37 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RootDirective.hpp"
#include <exception>
#include <iostream>

RootDirective::RootDirective(TokenisedBlock::const_iterator& cit) : SimpleDirective(cit)
{
	if (args.size() != 2)
		throw std::exception(); // root takes exactly one argument

	path = args[1];
	
	// append '/' to end of path if there isn't one
	if (path[path.size() - 1] != '/')
		path += '/';
}

void RootDirective::print(int depth) const
{
	for (int i = 0; i < depth; i++)
		std::cout << "  ";
	std::cout << "root " << path << ";" << std::endl;
}

const std::string& RootDirective::getPath() const { return path; }
