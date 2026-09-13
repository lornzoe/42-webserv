#include "AutoindexDirective.hpp"
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AutoindexDirective.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/08 04:41:26 by lyanga            #+#    #+#             */
/*   Updated: 2026/09/08 04:41:26 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "AutoindexDirective.hpp"
#include <stdexcept>
#include <iostream>

AutoindexDirective::AutoindexDirective(TokenisedBlock::const_iterator &cit) : SimpleDirective(cit)
{
    if (args.size() != 2)
        throw std::runtime_error("autoindex: requires 1 argument.");
    if (args[1] != "on" && args[1] != "off")
        throw std::runtime_error("autoindex directive expectes 'on'/'off' as an argument.");
    args[1] == "on" ? enabled = true : enabled = false;
}

void AutoindexDirective::print(int depth) const
{
	for (int i = 0; i < depth; i++)
		std::cout << "  ";
	std::cout << "autoindex "
        << (enabled ? "on" : "off")
        << ";" << std::endl;
}

bool AutoindexDirective::isEnabled() const
{
    return enabled;
}   
