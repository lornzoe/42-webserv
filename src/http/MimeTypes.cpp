/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MimeTypes.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/02 03:01:24 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/02 04:07:53 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "MimeTypes.hpp"
#include <map>

namespace {
    // For reference:
    // https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/MIME_types/Common_types
    // Not all MIME types are implemented here.

    const std::map<std::string, std::string>& getMimeMap()
    {
        static std::map<std::string, std::string> mimeMap;

        if (mimeMap.empty()) {
            // text
            mimeMap[".html"] = "text/html";
            mimeMap[".htm"]  = "text/html";
            mimeMap[".css"]  = "text/css";
            mimeMap[".csv"]  = "text/csv";
            mimeMap[".txt"]  = "text/plain";

            // application
            mimeMap[".js"]   = "application/javascript";
            mimeMap[".json"] = "application/json";
            mimeMap[".pdf"]  = "application/pdf";
            mimeMap[".xml"]  = "application/xml";
            mimeMap[".zip"]  = "application/zip";

            // image
            mimeMap[".gif"]  = "image/gif";
            mimeMap[".ico"]  = "image/x-icon";
            mimeMap[".jpg"]  = "image/jpeg";
            mimeMap[".jpeg"] = "image/jpeg";
            mimeMap[".png"]  = "image/png";
            mimeMap[".svg"]  = "image/svg+xml";

            // audio/video
            mimeMap[".mp3"]  = "audio/mpeg";
            mimeMap[".mp4"]  = "video/mp4";
        }
        return mimeMap;
    }
}

namespace MimeTypes {

    const std::string& forExtn(const std::string& extension)
    {
        static const std::string fallback = "application/octet-stream";
        const std::map<std::string, std::string>& m = getMimeMap();
        std::map<std::string, std::string>::const_iterator it = m.find(extension);

        if (it == m.end())
            return fallback;

        return it->second;
    }

    const std::string &forPath(const std::string &resource_path)
	{
		std::size_t dot = resource_path.rfind('.');
		if (dot == std::string::npos)
			return forExtn("");

		return forExtn(resource_path.substr(dot));
	}
}
