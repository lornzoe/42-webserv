/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/11 21:19:50 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/14 04:59:21 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include <iostream>

#include <sstream>
#include <string>
#include <vector>
#include <fstream>

int main(int argc, char** argv) {
	
	if (argc != 2)
	{
		std::cerr << "Usage: ./webserv <config_file>" << std::endl;
		return 1;
	}

	// Temporarily here to silence the compiler.
	(void)argc;
	(void)argv;

	// Main here.
	// std::cout << "This is where the webserv would've been, if I had one!"
	// 	<< std::endl;

	std::ifstream infile(argv[1]); 
	if (!infile) { 
        std::cerr << "Error: Could not open the file!" << std::endl;
        return 1;
    }

    std::string line;
	std::vector<std::string> lines;
    while (std::getline(infile, line)) {
		lines.push_back(line);
    }
    infile.close();

	// 0th pass because i forgot
	// verify {} correctness

	bool single_quotes = false;
	bool double_quotes = false;
	int indent = 0;
	for (std::size_t i = 0; i < lines.size(); ++i)
	{
    	for (std::string::const_iterator it = lines[i].begin(); it != lines[i].end(); ++it) 
		{
			switch(*it)
			{
				case '\'':
					single_quotes = !single_quotes;
					continue;
				case '\"':
					double_quotes = !double_quotes;
					continue;
				case '{':
					if (single_quotes || double_quotes)
						continue;
					indent++;
					continue;
				case '}':
					if (single_quotes || double_quotes)
						continue;
					if (indent == 0) // MAY WANT TO WRAP THE WHOLE THING IN TRY AND CATCH
						return 1; // REPLACE THIS WITH EXCEPTION
					indent--;
					continue;
				default:
					continue;
			}
		}
	}
	
	std::cout << "0th pass ok " << std::endl;

	// 1st pass
	// remove comments (everything after '#')
	// warning: keep in mind of quotation marks, not sure what else to keep in mind...

	single_quotes = false;
	double_quotes = false;
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
	std::cout << "1st pass ok " << std::endl;

	// 1.5th pass
	// reduce empty lines/ lines with only whitespace
	// warn: keep in mind of being in quotation marks

	single_quotes = false;
	double_quotes = false;
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
	std::cout << "1.5th pass ok " << std::endl;

	// 2nd pass
	// split based on tokens 
	// important: split { } ; from existing words

	std::vector<std::vector<std::string> > tokenised_lines;
	single_quotes = false;
	double_quotes = false;
	for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it)
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
	std::cout << "3rd pass ok" << std::endl;

	// 3rd pass
	// group directives together
	// one line is a directive
	// for now, add an indent every time theres a {, -1 indent when ending indent}
	
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
	std::cout << "3rd pass ok" << std::endl; 

	// printing

	std::cout << "=== result: ===" <<std::endl;
	int current_indent = 0;
	for (std::size_t i = 0; i < directives.size(); ++i)
	{
		bool is_closing = (directives[i].size() == 1 && directives[i][0] == "}");
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

		for (std::size_t j = 0; j < directives[i].size(); ++j)
		{
			std::cout << directives[i][j];
			if (j + 1 < directives[i].size())
			{
				if (directives[i][j + 1] != ";")
					std::cout << " ";
			}
		}
		std::cout << std::endl;

		if (!directives[i].empty() && directives[i].back() == "{")
		{
			current_indent++;
		}
	}
	return 0;
}