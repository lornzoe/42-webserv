/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Debug.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/11 22:09:17 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/11 22:09:58 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Debug.hpp"
#include <iostream>

void Debug::printIndent(int level)
{
	for (int i = 0; i < level; i++)
	{
		std::cout << ". . " << std::endl;
	}
}
