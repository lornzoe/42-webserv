/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/14 07:09:40 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/02 07:17:48 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "DirectiveRules.hpp"
#include "DirectiveFactory.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <stack>
#include <stdexcept>

namespace {
	bool checkBracesAndQuotations(const std::vector<std::string>& lines)
	{
		bool single_quotes = false;
		bool double_quotes = false;
		int indent = 0;
		for (std::vector<std::string>::const_iterator it = lines.begin(); it != lines.end(); ++it)
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
		return (!single_quotes && !double_quotes && indent == 0);
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
					// std::string s(1, c); // save it as its own string
					tokenised_line.push_back(std::string(1, c));
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

	bool validateDirectiveStrings(const std::vector<std::vector<std::string> >& lines)
	{
		// validate if the directives are in the right context.
		std::stack<DirectiveRules::Context> context_stack;
		context_stack.push(DirectiveRules::CONTEXT_MAIN);

		for (std::vector<std::vector<std::string> >::const_iterator cit = lines.begin(); cit != lines.end(); ++cit)
		{
			const std::string dir = cit->front();
			if (dir == "}")
			{
				context_stack.pop();
				continue;
			}

			DirectiveRules::Context current_context = context_stack.top();

			if (!DirectiveRules::canExist(dir))
				throw std::runtime_error("unknown directive '" + dir + "'");
			if (DirectiveRules::isSimpleType(dir) && cit->back() != ";")
				throw std::runtime_error("'" + dir + "': expected ';' terminator, got '" + cit->back() + "'");
			if (DirectiveRules::isBlockType(dir) && cit->back() != "{")
				throw std::runtime_error("'" + dir + "': expected '{' to open block, got '" + cit->back() + "'");
			if (!DirectiveRules::isValidInContext(dir, current_context))
				throw std::runtime_error("'" + dir + "' is not allowed in this context");

			if (DirectiveRules::isBlockType(dir))
			{
				if (dir == "server")
					context_stack.push(DirectiveRules::CONTEXT_SERVER);
				else if (dir == "location")
					context_stack.push(DirectiveRules::CONTEXT_LOCATION);
				else if (dir == "if")
					context_stack.push(DirectiveRules::CONTEXT_IF);
			}
		}
		return context_stack.top() == DirectiveRules::CONTEXT_MAIN;
	}
}

Config::Config(char *filename)
{
	// load the file
	std::ifstream infile(filename);

	if (!infile)
		throw std::runtime_error("Config: could not open file '" + std::string(filename) + "'");

	std::vector<std::string> file;
	{
		std::string line;
		while (std::getline(infile, line)) {
			file.push_back(line);
		}
	}
	infile.close();
	
	// strip the file of unnecessary content
    stripComments(file);
    stripEmptyLines(file);

	// check contents of file
    if (!checkBracesAndQuotations(file))
        throw std::runtime_error("Config: mismatched braces or quotes in file '" + std::string(filename) + "'");

	// tokenise it
    this->directives_string = generateTokenisedDirectives(file);

	// validate the tokens
	if (!validateDirectiveStrings(this->directives_string)) // many validation-specific exceptions are here.
		throw std::runtime_error("Config: unbalanced blocks in file '" + std::string(filename) + "' (a block was never closed)");

	/*
		TODO: Ensure parameters in config have:
			- valid parameters per directive
			- ensure each context has the minimum, correct number of directives
			- ensure information from multiple directives do not clash with each other (e.g. listen 0.0.0.0)
	*/

	// create the directives to fill up config
	Directive::TokenisedBlock::const_iterator cit = this->directives_string.begin();
	while (cit != this->directives_string.end())
		directives.push_back(DirectiveFactory::createDirective(cit));
}

Config::~Config()
{
	for (std::size_t i = 0; i < directives.size(); i++)
		delete directives[i];
}

const std::vector<Directive *>& Config::getDirectives() const
{
	return directives;
}

void Config::printDirectives() const
{
	for (std::size_t i = 0; i < directives.size(); i++)
		directives[i]->print(0);
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
			std::cout << "[" << directives_string[i][j] << "]";
		}
		std::cout << std::endl;

		if (!directives_string[i].empty() && directives_string[i].back() == "{")
		{
			current_indent++;
		}
	}
}
