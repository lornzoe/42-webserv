/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AutoindexDirective.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/08 04:41:45 by lyanga            #+#    #+#             */
/*   Updated: 2026/09/08 04:45:08 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef AUTOINDEXDIRECTIVE_HPP
#define AUTOINDEXDIRECTIVE_HPP

#include "SimpleDirective.hpp"

class AutoindexDirective : public SimpleDirective
{
    private:
        bool enabled;

    public:
        AutoindexDirective(TokenisedBlock::const_iterator& cit);
        void print(int depth) const;

        bool isEnabled() const;
};

#endif