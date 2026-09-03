/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ypua <ypua@student.42.singapore.sg>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/13 20:20:12 by ypua              #+#    #+#             */
/*   Updated: 2026/08/22 15:44:10 by ypua             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <unistd.h>
#include <sstream>

#ifndef SPACES
#define SPACES " \f\n\r\t\v"
#endif

namespace Utils
{
	void safeClose(int &fd);
	std::string ft_itoa(int num);
	std::string Trim(std::string s);
	std::string toLowercase(std::string str);
}

#endif
