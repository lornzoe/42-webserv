/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ypua <ypua@student.42.singapore.sg>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/13 20:18:55 by ypua              #+#    #+#             */
/*   Updated: 2026/08/22 15:44:13 by ypua             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils.hpp"
#include "FileDescriptor.hpp"
#include <sys/stat.h>

static void		LeftTrim(std::string &s)
{
	size_t found = s.find_first_not_of(OWS);
	if (found != std::string::npos)
		s.erase(0, found);
	else
		s.clear();
}

static void		RightTrim(std::string &s)
{
	size_t found = s.find_last_not_of(OWS);
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

std::string Utils::toLowercase(std::string str)
{
	for (size_t j = 0; j < str.length(); j++)
		str[j] = static_cast<char>(tolower(static_cast<unsigned char>(str[j])));
	return str;
}

bool	Utils::canFind(char c, std::string const &str) {
	return str.find(c) != std::string::npos;
}

bool		Utils::split_pair(std::string const &str, char sep, kvpair &kvp)
{
	size_t	sep_idx = 0;
	
	if ((sep_idx = str.find(sep)) == std::string::npos)
		return false;
	
	kvp.first = str.substr(0, sep_idx);
	kvp.second = str.substr(sep_idx + 1);
	return true;
}

std::vector<std::string>	Utils::ft_split(
	std::string const &str, std::string const &delim, bool ignoreEmpty)
{
	size_t	cursr = 0;
	size_t	index = 0;
	size_t	dSize = delim.size();
	std::vector<std::string>	str_vect;

	if (dSize == 0)
		return str_vect.push_back(str), str_vect;

	while ((index = str.find(delim, cursr)) != std::string::npos)
	{
		size_t	len = index - cursr;
		if (len != 0 || !ignoreEmpty)
			str_vect.push_back(str.substr(cursr, len));
		cursr = index + dSize;
	}
	if (cursr < str.size())
		str_vect.push_back(str.substr(cursr));
	else if (!ignoreEmpty)
		str_vect.push_back("");
	return str_vect;
}

bool	Utils::readFile(const std::string &path, std::string &body)
{
	FileDescriptor file(path);
	if (file.get() == -1)
		return false;

	struct stat st;
	if (stat(path.c_str(), &st) == -1)
		return false;

	body.resize(st.st_size);

	ssize_t total = 0;
	while (total < st.st_size)
	{
		ssize_t bytes = read(file.get(), &body[total], st.st_size - total);
		if (bytes <= 0)
			return false;
		total += bytes;
	}
	return true;
}
