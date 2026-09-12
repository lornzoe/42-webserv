/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/13 20:20:12 by ypua              #+#    #+#             */
/*   Updated: 2026/09/12 19:58:27 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <unistd.h>
#include <sstream>
#include <vector>

#ifndef OWS
#define OWS " \t"
#endif

namespace Utils
{
	void safeClose(int &fd);
	std::string ft_itoa(int num);
	std::string Trim(std::string s);
	std::string toLowercase(std::string str);

	bool	canFind(char c, std::string const &str);

	typedef	std::pair<std::string, std::string>		kvpair;
	bool	split_pair(std::string const &str, char delim, kvpair &kvp);

	std::vector<std::string>	ft_split(
									std::string const &str,
									std::string const &delim,
									bool ignoreEmpty = true);

	bool	readFile(const std::string &path, std::string &body);
	bool	saveFile(std::string const &dirPath, std::string const &fname, std::string const &content);

	bool	normaliseUri(const std::string &uri, std::string &out);
}

#endif
