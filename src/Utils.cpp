/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ypua <ypua@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/13 20:18:55 by ypua              #+#    #+#             */
/*   Updated: 2026/08/13 20:25:52 by ypua             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils.hpp"

void LeftTrim(std::string &s)
{
	size_t found = s.find_first_not_of(SPACES);
	if (found != std::string::npos)
		s.erase(0, found);
	else
		s.clear();
}

void RightTrim(std::string &s)
{
	size_t found = s.find_last_not_of(SPACES);
	if (found != std::string::npos)
		s.erase(found + 1);
	else
		s.clear();
}

std::string Utils::Trim(std::string s)
{
	LeftTrim(s);
	RightTrim(s);
	return s;
}

void Utils::safeClose(int &fd)
{
	if (fd == -1)
		return;
	close(fd);
	fd = -1;
}

std::string Utils::ft_itoa(int num)
{
	std::stringstream ss;
	ss << num;
	return (ss.str());
}