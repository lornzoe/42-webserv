/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpStat.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 04:34:58 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/28 12:43:50 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPSTAT_HPP
#define HTTPSTAT_HPP

#include <string>

static const std::string HEADER_END = "\r\n\r\n";
static const std::string CRLF = "\r\n";

namespace HttpStat {

    const std::string& getReason(int code);
    bool isKnown(int code);

} // namespace HttpStat

#endif