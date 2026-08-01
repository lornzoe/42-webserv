/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RootDirective.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 00:00:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/02 07:18:30 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RootDirective.hpp"
#include <stdexcept>
#include <iostream>

RootDirective::RootDirective(TokenisedBlock::const_iterator& cit) : SimpleDirective(cit)
{
	if (args.size() != 2)
		throw std::runtime_error("root: expects exactly one argument (e.g. 'root /var/www;')");

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
