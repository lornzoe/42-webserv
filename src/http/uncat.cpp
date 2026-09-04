#include "ServerDirective.hpp"
#include "ListenDirective.hpp"

#include <string>
#include <fstream>
#include <sys/stat.h>
#include "Utils.hpp"

bool			saveFile(std::string const &dirPath, std::string const &fname, std::string const &content)
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
