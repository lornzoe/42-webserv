/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/13 20:18:55 by ypua              #+#    #+#             */
/*   Updated: 2026/09/13 03:26:55 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils.hpp"
#include "FileDescriptor.hpp"

#include <fstream>
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

bool	Utils::saveFile(std::string const &dirPath, std::string const &fname, std::string const &content)
{
	std::string		filename;
	std::string		fullPath;

	if (fname.find("/") != std::string::npos || fname == "." || fname == "..")
		return false;
	if (fname.empty())
		filename = "upload";
	else
		filename = fname;

	struct stat		stmp;
	int				serial = 1;
	bool			dot = false, dot_check = false;
	size_t			dot_pos;
	fullPath = dirPath + '/' + filename;
	while (!stat(fullPath.c_str(), &stmp))
	{
		if (!dot_check)
		{
			dot_check = true;
			if (filename.find('.') != std::string::npos)
			{
				dot = true;
				dot_pos = filename.find_last_of('.');
			}
		}

		std::string		tmp_fname = filename;
		std::string		suffix = "(" + Utils::ft_itoa(serial++) + ")";
		if (dot && dot_pos != 0)
			tmp_fname.insert(dot_pos, suffix);
		else
			tmp_fname += suffix;
		fullPath = dirPath + '/' + tmp_fname;
	}

	std::ofstream	ofs(fullPath.c_str(), std::ios::trunc | std::ios::binary);
	ofs.write(content.data(), content.size());
	return (!ofs.fail());
}

namespace {
	int	hexVal(char c)
	{
		if (c >= '0' && c <= '9')
			return c - '0';
		if (c >= 'a' && c <= 'f')
			return c - 'a' + 10;
		if (c >= 'A' && c <= 'F')
			return c - 'A' + 10;
		return -1;
	}

	// decdes %-encoded chars in str (e.g. %20 -> ' ')
	bool	percentDecode(std::string const &in, std::string &out)
	{
		out.clear();
		out.reserve(in.size());

		for (size_t i = 0; i < in.size(); i++)
		{
			if (in[i] != '%')
			{
				out += in[i];
				continue;
			}

			if (i + 2 >= in.size())
				return false;

			int	hi = hexVal(in[i + 1]);
			int	lo = hexVal(in[i + 2]);
			if (hi < 0 || lo < 0)
				return false;

			char	c = static_cast<char>(hi * 16 + lo);
			if (c == '\0')
				return false;

			out += c;
			i += 2;
		}
		return true;
	}
}

// guarantee output is a valid path
// returns false if the input is malformed or escapes document root
bool	Utils::normaliseUri(const std::string &uri, std::string &out)
{
	out.clear();
	if (uri.empty() || uri[0] != '/') // absolute path check
		return false;

	// convert uri to decoded cform
	std::string	decoded;
	if (!percentDecode(uri, decoded)) 
		return false;

	// check if decoded is still valid
	if (decoded.empty() || decoded[0] != '/')
		return false;
	for (size_t i = 0; i < decoded.size(); i++)
	{
		unsigned char	c = static_cast<unsigned char>(decoded[i]);
		if (c < 0x20 || c == 0x7F) // control chars check
			return false;
	}

	// split uri into segments and resolve . and .. segments
	std::vector<std::string> segments = Utils::ft_split(decoded, "/"); 
	bool	trailingSlash = (decoded[decoded.size() - 1] == '/');
	if (!segments.empty()
		&& (segments[segments.size() - 1] == "." || segments[segments.size() - 1] == ".."))
		trailingSlash = true;

	// using vector instead of stack to help with debugging
	std::vector<std::string> stack;
	for (size_t i = 0; i < segments.size(); i++)
	{
		if (segments[i] == ".")
			continue;
		if (segments[i] == "..")
		{
			if (stack.empty())
				return false; // escapes the document root
			stack.pop_back();
			continue;
		}
		stack.push_back(segments[i]);
	}

	if (stack.empty())
		return (out = "/", true);

	for (size_t i = 0; i < stack.size(); i++)
	{
		out += '/';
		out += stack[i];
	}
	if (trailingSlash)
		out += '/';
	return true;
}
