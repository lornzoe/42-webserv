/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/14 07:09:40 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/10 01:38:41 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "DirectiveRules.hpp"
#include "DirectiveFactory.hpp"
#include "QuoteTracker.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <stack>
#include <stdexcept>

namespace {
	bool checkBracesAndQuotations(const std::vector<std::string>& lines)
	{
		QuoteTracker tracker;
		int indent = 0;
		for (std::vector<std::string>::const_iterator it = lines.begin(); it != lines.end(); ++it)
		{
			tracker.newLine();
			for (std::string::const_iterator cit = it->begin(); cit != it->end(); ++cit)
			{
				if (!tracker.feed(*cit))
					continue;

				if (*cit == '{')
				{
					if (tracker.inQuotes())
						continue;
					indent++;
				}
				else if (*cit == '}')
				{
					if (tracker.inQuotes())
						continue;
					if (indent == 0)
						return false;
					indent--;
				}
			}
		}
		return (!tracker.inQuotes() && indent == 0);
	}

	void stripComments(std::vector<std::string>& lines)
	{
		for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it)
		{
			QuoteTracker tracker;
			std::size_t pos = std::string::npos;
			std::size_t idx = 0;
			for (std::string::const_iterator cit = it->begin(); cit != it->end(); ++cit, ++idx)
			{
				if (!tracker.feed(*cit))
					continue;

				if (*cit == '#')
				{
					if (tracker.inQuotes())
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
		for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); )
		{
			QuoteTracker tracker;
			bool is_empty = true;
			for (std::string::const_iterator cit = it->begin(); cit != it->end(); ++cit)
			{
				if (!tracker.feed(*cit))
				{
					is_empty = false;
					continue;
				}
				if (!std::isspace(static_cast<unsigned char>(*cit)) || tracker.inQuotes())
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
		for (std::vector<std::string>::const_iterator it = lines.begin(); it != lines.end(); ++it)
		{
			QuoteTracker tracker;
			std::vector<std::string> tokenised_line;
			std::string current_token;
			// iterate by the character
			for (std::string::const_iterator cit = it->begin(); cit != it->end(); ++cit)
			{
				char c = *cit;
				if (!tracker.feed(c))
					continue;

				if (tracker.inQuotes() || c == '\'' || c == '"')
					current_token += c;
				else if (std::isspace(static_cast<unsigned char>(c)))
				{
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

Config::Config() : initialised(false)
{
}

Config& Config::getInstance()
{
	static Config instance;
	return instance;
}

void Config::init(const std::string& filename)
{
	Config& instance = getInstance();

	if (instance.initialised)
		throw std::runtime_error("Config: already initialised");

	try
	{
		instance.load(filename);
	}
	catch (...)
	{
		instance.clear();
		throw;
	}
	instance.initialised = true;
}

bool Config::isInitialised() const
{
	return initialised;
}

void Config::load(const std::string& filename)
{
	// load the file
	std::ifstream infile(filename.c_str());

	if (!infile)
		throw std::runtime_error("Config: could not open file '" + filename + "'");

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
        throw std::runtime_error("Config: mismatched braces or quotes in file '" + filename + "'");

	// tokenise it
    this->directives_string = generateTokenisedDirectives(file);

	// validate the tokens
	if (!validateDirectiveStrings(this->directives_string)) // many validation-specific exceptions are here.
		throw std::runtime_error("Config: unbalanced blocks in file '" + filename + "' (a block was never closed)");

	/*
		TODO: Ensure parameters in config have:
			- valid parameters per directive
			- ensure each context has the minimum, correct number of directives
			- ensure information from multiple directives do not clash with each other (e.g. listen 0.0.0.0)
		update: checks are handled on each directive constructor. 
				block directives check there is at most/at least x directive as a direct child in their block.

		TODO: ensure info from multiple directives do not clash with each other
		(e.g. listen 0.0.0.0 vs listen 127.0.0.1 on the same port, or two locations sharing a prefix).
		
		These span server blocks, so they should be implemented here in Config
		rather than in a single directive.
		hasOverlappingBind() in main.cpp is the existing function to reference for this.

	*/

	// create the directives to fill up config
	Directive::TokenisedBlock::const_iterator cit = this->directives_string.begin();
	while (cit != this->directives_string.end())
		directives.push_back(DirectiveFactory::createDirective(cit));
}

Config::~Config()
{
	clear();
}

void Config::clear()
{
	for (std::size_t i = 0; i < directives.size(); i++)
		delete directives[i];
	directives.clear();
	directives_string.clear();
}

const std::vector<Directive *>& Config::getDirectives() const
{
	if (!initialised)
		throw std::runtime_error("Config: not initialised (call Config::init() first)");
	return directives;
}

std::vector<const ServerDirective *> Config::getServerDirectives() const
{
	if (!initialised)
		throw std::runtime_error("Config: not initialised (call Config::init() first)");

	std::vector<const ServerDirective *> servers;
	for (std::size_t i = 0; i < directives.size(); i++)
	{
		const ServerDirective* server = dynamic_cast<const ServerDirective *>(directives[i]);
		if (server)
			servers.push_back(server);
	}
	return servers;
}

void Config::printDirectives() const
{
	if (!initialised)
		throw std::runtime_error("Config: not initialised (call Config::init() first)");
	for (std::size_t i = 0; i < directives.size(); i++)
		directives[i]->print(0);
}

void Config::printConfig() const
{
	if (!initialised)
		throw std::runtime_error("Config: not initialised (call Config::init() first)");

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
