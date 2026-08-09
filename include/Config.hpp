/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 16:41:56 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/10 01:38:17 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <vector>
#include <string>

#include "Directive.hpp"
#include "ServerDirective.hpp"

class Config
{
	private:
		std::vector<std::vector<std::string> > directives_string; // for debugging
		std::vector<Directive *> directives; // parsed top-level directives (e.g. server blocks)
		bool initialised;

		Config();
		~Config();
		Config(const Config& other);            // not implemented
		Config& operator=(const Config& other); // not implemented

		void load(const std::string& filename);
		void clear();

	public:
		static Config& getInstance();
		static void init(const std::string& filename);

		bool isInitialised() const;

		void printConfig() const;
		void printDirectives() const;

		const std::vector<Directive *>& getDirectives() const;
		std::vector<const ServerDirective *> getServerDirectives() const;

};

#endif