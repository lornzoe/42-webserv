/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BlockDirective.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/11 23:12:29 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/01 19:15:12 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BLOCKDIRECTIVE_HPP
#define BLOCKDIRECTIVE_HPP

#include "Directive.hpp"
#include <string>
#include <vector>
#include <cstddef>

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
};

#endif
