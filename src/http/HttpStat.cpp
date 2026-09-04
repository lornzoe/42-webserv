/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpStat.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 04:34:58 by lyanga            #+#    #+#             */
/*   Updated: 2026/07/28 13:00:40 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpStat.hpp"
#include <map>

namespace {
    // For reference:
    // https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Status
    // Not all status codes are implemented here.

    const std::map<int, std::string>& getStatusMap()
    {
        static std::map<int, std::string> statusMap;

        if (statusMap.empty()) {
            // 1xx
            statusMap[100] = "Continue";
            statusMap[101] = "Switching Protocols";

            // 2xx
            statusMap[200] = "OK";
            statusMap[201] = "Created";
            statusMap[202] = "Accepted";
            statusMap[204] = "No Content";
            statusMap[206] = "Partial Content";

            // 3xx
            statusMap[300] = "Multiple Choices";
            statusMap[301] = "Moved Permanently";
            statusMap[302] = "Found";
            statusMap[303] = "See Other";
            statusMap[304] = "Not Modified";
            statusMap[307] = "Temporary Redirect";
            statusMap[308] = "Permanent Redirect";

            // 4xx
            statusMap[400] = "Bad Request";
            statusMap[401] = "Unauthorized";
            statusMap[403] = "Forbidden";
            statusMap[404] = "Not Found";
            statusMap[405] = "Method Not Allowed";
            statusMap[406] = "Not Acceptable";
            statusMap[408] = "Request Timeout";
            statusMap[409] = "Conflict";
            statusMap[411] = "Length Required";
            statusMap[413] = "Payload Too Large";
            statusMap[414] = "URI Too Long";
            statusMap[415] = "Unsupported Media Type";
            statusMap[417] = "Expectation Failed";
            statusMap[431] = "Request Header Fields Too Large";

            // 5xx
            statusMap[500] = "Internal Server Error";
            statusMap[501] = "Not Implemented";
            statusMap[502] = "Bad Gateway";
            statusMap[503] = "Service Unavailable";
            statusMap[504] = "Gateway Timeout";
            statusMap[505] = "HTTP Version Not Supported";
        }
        return statusMap;
    }
}

namespace HttpStat {

    const std::string& getReason(int code)
    {
        static const std::string unknown = "Unknown";
        const std::map<int, std::string>& m = getStatusMap();
        std::map<int, std::string>::const_iterator it = m.find(code);

        if (it == m.end())
            return unknown;

        return it->second;
    }

    bool isKnown(int code)
    {
        const std::map<int, std::string>& m = getStatusMap();
        return m.find(code) != m.end();
    }

}
