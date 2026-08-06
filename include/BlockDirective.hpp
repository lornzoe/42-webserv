/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BlockDirective.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/11 23:12:29 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/06 02:18:04 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BLOCKDIRECTIVE_HPP
#define BLOCKDIRECTIVE_HPP

#include "Directive.hpp"
#include <string>
#include <vector>
#include <cstddef>
#include <stdexcept>

class BlockDirective : public Directive
{
	protected:
		std::vector<Directive *> directives; // list of directives in block
	public:
		BlockDirective(TokenisedBlock::const_iterator& cit);
		virtual ~BlockDirective();	
		virtual void print(int depth) const = 0;
		const std::vector<Directive *>& getChildren() const;

	protected:
		template <typename T>
		std::vector<const T *> getChildren() const
		{
			std::vector<const T *> result;
			for (std::size_t i = 0; i < directives.size(); i++)
			{
				const T* match = dynamic_cast<const T *>(directives[i]);
				if (match)
					result.push_back(match);
			}
			return result;
		}

		template <typename T>
		const T* getChild() const
		{
			for (std::size_t i = 0; i < directives.size(); i++)
			{
				const T* match = dynamic_cast<const T *>(directives[i]);
				if (match)
					return match;
			}
			return NULL;
		}

		// Checks for a block's own contents. 
		// Like getChild/getChildren, these only see direct children.
		// We'll see if this suffices.
		template <typename T>
		void requireAtMostOne(const std::string& blockName, const std::string& directiveName) const
		{
			if (getChildren<T>().size() > 1)
				throw std::runtime_error(blockName + ": duplicate '" + directiveName
										 + "' directive (only one allowed per block)");
		}

		template <typename T>
		void requireAtLeastOne(const std::string& blockName, const std::string& directiveName) const
		{
			if (getChildren<T>().empty())
				throw std::runtime_error(blockName + ": requires at least one '"
										 + directiveName + "' directive");
		}
};

#endif
