/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   QuoteTracker.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/02 09:04:56 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/02 09:06:20 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "QuoteTracker.hpp"

QuoteTracker::QuoteTracker() : single_quotes(false), double_quotes(false), escaped(false)
{
}

void QuoteTracker::newLine()
{
    escaped = false;
}

bool QuoteTracker::inQuotes() const
{
    return single_quotes || double_quotes;
}

bool QuoteTracker::feed(char c)
{
    if (escaped)
    {
        escaped = false;
        return true;
    }
    if (c == '\\' && inQuotes())
    {
        escaped = true;
        return false;
    }
    if (c == '\'')
    {
        if (!double_quotes)
            single_quotes = !single_quotes;
    }
    else if (c == '"')
    {
        if (!single_quotes)
            double_quotes = !double_quotes;
    }
    return true;
}