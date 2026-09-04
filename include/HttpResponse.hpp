/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/20 17:39:57 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/21 04:29:31 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>

class ServerDirective;

namespace HttpResponse 
{
	std::string		build(int code, const std::string &contentType, const std::string &body,
		const std::string additionalHeaders = "");
	std::string		buildError(int code, const std::string &uri, ServerDirective const *servDir);
	std::string 	defaultErrorBody(int code);
};

#endif