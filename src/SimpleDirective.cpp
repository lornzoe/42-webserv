/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SimpleDirective.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/11 22:25:23 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/11 23:10:57 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SimpleDirective.hpp"

SimpleDirective::SimpleDirective() {}

SimpleDirective::SimpleDirective(char *raw_arg): raw_arg(raw_arg) {}

SimpleDirective::SimpleDirective(std::string raw_arg): raw_arg(raw_arg) {}

SimpleDirective::~SimpleDirective() {}