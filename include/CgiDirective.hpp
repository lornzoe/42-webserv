/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiDirective.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/01 00:00:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/11/01 00:00:00 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIDIRECTIVE_HPP
#define CGIDIRECTIVE_HPP

#include "SimpleDirective.hpp"
#include <string>
#include <vector>

class CgiDirective : public SimpleDirective
{
	private:
		std::string extension;
		std::string runner;

	public:
		CgiDirective(TokenisedBlock::const_iterator& cit);
		void print(int depth) const;

		const std::string& getExtension() const;
		const std::string& getRunner() const;

		// true if the last segment of path ends in this directive's extension
		bool matches(const std::string& path) const;

		// shared by ServerDirective and LocationDirective
		static const CgiDirective* find(const std::vector<const CgiDirective *>& cgis, const std::string& path);
		static void requireUniqueExtensions(const std::vector<const CgiDirective *>& cgis, const std::string& blockName);
};

#endif
