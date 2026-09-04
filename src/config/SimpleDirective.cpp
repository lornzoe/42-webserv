/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SimpleDirective.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/11 22:25:23 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/24 09:48:26 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SimpleDirective.hpp"

SimpleDirective::SimpleDirective(Directive::TokenisedBlock::const_iterator& cit) : Directive(*cit)
{
	cit++; // a simple directive occupies exactly one line
}

SimpleDirective::~SimpleDirective() {}
