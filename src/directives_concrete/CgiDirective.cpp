/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiDirective.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/01 00:00:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/11/01 00:00:00 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiDirective.hpp"
#include "Utils.hpp"

#include <stdexcept>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>

CgiDirective::CgiDirective(TokenisedBlock::const_iterator& cit) : SimpleDirective(cit)
{
	if (args.size() != 3) // name + extension + runner
		throw std::runtime_error("cgi: expects an extension and a runner (e.g. 'cgi .py /usr/bin/python3;')");

	const std::string& ext = args[1];
	if (ext.size() < 2 || ext[0] != '.')
		throw std::runtime_error("cgi: extension must start with '.' (got '" + ext + "')");
	if (ext.find('.', 1) != std::string::npos || ext.find('/') != std::string::npos)
		throw std::runtime_error("cgi: extension must not contain '.' or '/' after the leading dot (got '" + ext + "')");

	// the CGI may be run from the script's directory, so a relative runner would break
	const std::string& run = args[2];
	if (run.empty() || run[0] != '/')
		throw std::runtime_error("cgi: runner must be an absolute path (got '" + run + "')");

	struct stat st;
	if (stat(run.c_str(), &st) != 0 || !S_ISREG(st.st_mode))
		throw std::runtime_error("cgi: runner does not exist or is not a regular file (got '" + run + "')");
	if (access(run.c_str(), X_OK) != 0)
		throw std::runtime_error("cgi: runner is not executable (got '" + run + "')");

	extension = ext;
	runner = run;
}

void CgiDirective::print(int depth) const
{
	for (int i = 0; i < depth; i++)
		std::cout << "  ";
	std::cout << "cgi " << extension << " " << runner << ";" << std::endl;
}

const std::string& CgiDirective::getExtension() const { return extension; }
const std::string& CgiDirective::getRunner() const { return runner; }

bool CgiDirective::matches(const std::string& path) const
{
	return Utils::getExtension(path) == extension;
}

const CgiDirective* CgiDirective::find(const std::vector<const CgiDirective *>& cgis, const std::string& path)
{
	for (std::size_t i = 0; i < cgis.size(); i++)
	{
		if (cgis[i]->matches(path))
			return cgis[i];
	}
	return NULL;
}

void CgiDirective::requireUniqueExtensions(const std::vector<const CgiDirective *>& cgis, const std::string& blockName)
{
	for (std::size_t i = 0; i < cgis.size(); i++)
	{
		for (std::size_t j = i + 1; j < cgis.size(); j++)
		{
			if (cgis[i]->getExtension() == cgis[j]->getExtension())
				throw std::runtime_error(blockName + ": duplicate 'cgi' directive for extension '"
										 + cgis[i]->getExtension() + "'");
		}
	}
}
