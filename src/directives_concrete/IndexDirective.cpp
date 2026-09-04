/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IndexDirective.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 00:00:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/02 07:18:11 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IndexDirective.hpp"
#include <stdexcept>
#include <iostream>

IndexDirective::IndexDirective(TokenisedBlock::const_iterator& cit) : SimpleDirective(cit)
{
	if (args.size() < 2)
		throw std::runtime_error("index: requires at least one file argument");

	for (std::size_t i = 1; i < args.size(); i++)
		files.push_back(args[i]);
}

void IndexDirective::print(int depth) const
{
	for (int i = 0; i < depth; i++)
		std::cout << "  ";
	std::cout << "index";
	for (std::size_t i = 0; i < files.size(); i++)
		std::cout << " " << files[i];
	std::cout << ";" << std::endl;
}

const std::vector<std::string>& IndexDirective::getFiles() const { return files; }
