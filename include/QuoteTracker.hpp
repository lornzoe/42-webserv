/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   QuoteTracker.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/02 09:02:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/02 09:07:05 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef QUOTETRACKER_HPP
#define QUOTETRACKER_HPP

// helper class for config class

class QuoteTracker
{
    private:
        bool single_quotes;
        bool double_quotes;
        bool escaped;
    public:
        QuoteTracker();
        
        void newLine();
        bool inQuotes() const;
        bool feed(char c);
};

#endif
