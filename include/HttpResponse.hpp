/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ypua <ypua@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/20 17:39:57 by lyanga            #+#    #+#             */
/*   Updated: 2026/10/29 20:15:29 by ypua             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>

class ServerDirective;

namespace HttpResponse
{
	std::string build(int code, const std::string &contentType, const std::string &body,
					  const std::string additionalHeaders = "");
	std::string buildError(int code, const std::string &uri, ServerDirective const *servDir);
	std::string buildAutoindex(const std::string &fsDir, const std::string &uriPath, const ServerDirective *servDir);
	std::string defaultErrorBody(int code);
	std::string buildCgiResponse(int code, std::string const &headers, std::string const &body);
};

#endif