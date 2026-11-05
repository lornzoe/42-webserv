/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ypua <ypua@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/13 20:20:12 by ypua              #+#    #+#             */
/*   Updated: 2026/11/05 18:34:11 by ypua             ###   ########.fr       */
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

	template <typename T>
	std::string to_string(const T &val)
	{
		std::stringstream ss;
		ss << val;
		return ss.str();
	}

	std::string Trim(std::string s);
	std::string toLowercase(std::string str);

	bool canFind(char c, std::string const &str);

	typedef std::pair<std::string, std::string> kvpair;
	bool split_pair(std::string const &str, char delim, kvpair &kvp);

	std::vector<std::string> ft_split(
		std::string const &str,
		std::string const &delim,
		bool ignoreEmpty = true);

	bool readFile(const std::string &path, std::string &body);
	bool saveFile(std::string const &dirPath, std::string const &fname, std::string const &content);

	bool	normaliseUri(const std::string &uri, std::string &out);

	// extension of the last path segment including the dot, e.g. ".py"; empty if none
	std::string	getExtension(const std::string &path);
}

#endif
