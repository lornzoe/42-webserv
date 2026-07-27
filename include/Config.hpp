/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 16:41:56 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/27 12:45:09 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <vector>
#include <string>

#include "Directive.hpp"

class Config
{
	private:
		std::vector<std::vector<std::string> > directives_string; // for debugging
		std::vector<Directive *> directives; // parsed top-level directives (e.g. server blocks)
	public:
		Config(char *infile);
		~Config();

		void printConfig();
		void printDirectives() const;

};

#endif