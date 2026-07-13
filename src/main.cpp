/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/11 21:19:50 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/13 21:26:03 by lyanga           ###   ########.fr       */
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
			if (*cit == '\'' && !double_quotes)
				single_quotes = !single_quotes;
			else if (*cit == '"' && !single_quotes)
				double_quotes = !double_quotes;
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
			if (*cit == '\'' && !double_quotes)
				single_quotes = !single_quotes;
			else if (*cit == '"' && !single_quotes)
				double_quotes = !double_quotes;
			if (!std::isspace(static_cast<unsigned char>(*cit)) || single_quotes || double_quotes)
				is_empty = false;
		}
		if (is_empty)
			it = lines.erase(it);
		else
			++it;
	}

	// 2nd pass
	// split based on tokens 
	// important: split { } ; from existing words


	// 3rd pass
	// group directives together
	// one line is a directive
	// for now, add an indent every time theres a {, -1 indent when ending indent}
	

	// output

	for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it)
	{
		std::cout << *it << std::endl;
	}
	return 0;
}