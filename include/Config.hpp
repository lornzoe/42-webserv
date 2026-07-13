/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 16:41:56 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/14 07:41:27 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <vector>
#include <string>

class Config
{
	private:
		std::vector<std::vector<std::string> > directives_string; // for debugging
	public:
		Config(char *infile);
		~Config();

		void printConfig();

};

#endif