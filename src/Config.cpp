/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/14 07:09:40 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/14 17:18:00 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include <iostream>
#include <sstream>
#include <fstream>

namespace {
	std::vector<std::string> loadFile(char *file)
	{
		std::ifstream infile(file); 
		if (!infile) { 
			std::cerr << "Error: Could not open the file!" << std::endl;
			throw std::exception(); // replace this with a better exception
		}

		std::string line;
		std::vector<std::string> lines;
		while (std::getline(infile, line)) {
			lines.push_back(line);
		}
		infile.close();

		return lines;
	}

	bool checkBraces(std::vector<std::string> lines)
	{
		bool single_quotes = false;
		bool double_quotes = false;
		int indent = 0;
		for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it)
		{
			for (std::string::const_iterator cit = it->begin(); cit != it->end(); ++cit)
			{
				if (*cit == '\'')
				{
					if (!double_quotes)
						single_quotes = !single_quotes;
				}	
				else if (*cit == '"')
				{
					if (!single_quotes)
						double_quotes = !double_quotes;
				}
				else if (*cit == '{')
				{
					if (single_quotes || double_quotes)
							continue;
					indent++;
				}
				else if (*cit == '}')
				{
					if (single_quotes || double_quotes)
						continue;
					if (indent == 0)
						return false;
					indent--;
				}
			}
		}
		return true;
	}

	void stripComments(std::vector<std::string>& lines)
	{
		bool single_quotes = false;
		bool double_quotes = false;
		for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it)
		{
			std::size_t pos = std::string::npos;
			std::size_t idx = 0;
			for (std::string::const_iterator cit = it->begin(); cit != it->end(); ++cit)
			{
				if (*cit == '\'')
				{
					if (!double_quotes)
						single_quotes = !single_quotes;
				}	
				else if (*cit == '"')
				{
					if (!single_quotes)
						double_quotes = !double_quotes;
				}
				if (*cit == '#')
				{
					if (single_quotes || double_quotes)
							continue;
					pos = idx;
					break;
				}
			}
			if (pos != std::string::npos)
				it->erase(pos);
		}
	}

	void stripEmptyLines(std::vector<std::string>& lines)
	{
		bool single_quotes = false;
		bool double_quotes = false;
		for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); )
		{
			bool is_empty = true;
			for (std::string::const_iterator cit = it->begin(); cit != it->end(); ++cit)
			{
				if (*cit == '\'')
				{
					if (!double_quotes)
						single_quotes = !single_quotes;
				}	
				else if (*cit == '"')
				{
					if (!single_quotes)
						double_quotes = !double_quotes;
				}
				if (!std::isspace(static_cast<unsigned char>(*cit)) || single_quotes || double_quotes)
					is_empty = false;
			}
			if (is_empty)
				it = lines.erase(it);
			else
				++it;
		}
	}

	std::vector<std::vector<std::string> > generateTokenisedDirectives(const std::vector<std::string>& lines)
	{
		std::vector<std::vector<std::string> > tokenised_lines;
		bool single_quotes = false;
		bool double_quotes = false;
		for (std::vector<std::string>::const_iterator it = lines.begin(); it != lines.end(); ++it)
		{
			std::vector<std::string> tokenised_line;
			std::string current_token;
			// iterate by the character
			for (std::string::const_iterator cit = it->begin(); cit != it->end(); ++cit)
			{
				char c = *cit;
				if (c == '\'')
				{
					if (!double_quotes)
						single_quotes = !single_quotes;
					current_token += c;
				}
				else if (c == '"')
				{
					if (!single_quotes)
						double_quotes = !double_quotes;
					current_token += c;
				}
				else if (single_quotes || double_quotes) // everything stays together in quotes
				{
					current_token += c;
				}
				else if (std::isspace(static_cast<unsigned char>(c))) 
				{
					// if whitespace, push token and clear current
					// keep the 2 conditions seperate so that multiple whitespace in a row get caught
					if (!current_token.empty())
					{
						tokenised_line.push_back(current_token);
						current_token.clear();
					}
				}
				else if (c == '{' || c == '}' || c == ';')
				{
					if (!current_token.empty())
					{
						tokenised_line.push_back(current_token);
						current_token.clear();
					}
					std::string s(1, c); // save it as its own string
					tokenised_line.push_back(s);
				}
				else
					current_token += c;
			}
			if (!current_token.empty())
				tokenised_line.push_back(current_token);
			if(!tokenised_line.empty())
				tokenised_lines.push_back(tokenised_line);
		}

		std::vector<std::vector<std::string> > directives;
		std::vector<std::string> current_directive;
		
		for (std::size_t i = 0; i < tokenised_lines.size(); ++i)
		{
			for (std::size_t j = 0; j < tokenised_lines[i].size(); ++j)
			{
				std::string token = tokenised_lines[i][j];
				current_directive.push_back(token);
				

				if (token == ";" || token == "{" || token == "}")
				{
					directives.push_back(current_directive);
					current_directive.clear();
				}
			}
		}
		if (!current_directive.empty())
		{
			directives.push_back(current_directive);
		}
		return directives;
	}
}

Config::Config(char *infile)
{
    std::vector<std::string> file = loadFile(infile);
    if (!checkBraces(file))
        throw std::exception(); // replace with better defined exception
    stripComments(file);
    stripEmptyLines(file);

    this->directives_string = generateTokenisedDirectives(file);
}

Config::~Config()
{
}

void Config::printConfig()
{
    int current_indent = 0;
	for (std::size_t i = 0; i < directives_string.size(); ++i)
	{
		bool is_closing = (directives_string[i].size() == 1 && directives_string[i][0] == "}");
		if (is_closing)
		{
			current_indent--;
			if (current_indent < 0)
				current_indent = 0;
		}

		for (int k = 0; k < current_indent; ++k)
		{
			std::cout << "    ";
		}

		for (std::size_t j = 0; j < directives_string[i].size(); ++j)
		{
			std::cout << directives_string[i][j];
			if (j + 1 < directives_string[i].size())
			{
				if (directives_string[i][j + 1] != ";")
					std::cout << " ";
			}
		}
		std::cout << std::endl;

		if (!directives_string[i].empty() && directives_string[i].back() == "{")
		{
			current_indent++;
		}
	}
}
