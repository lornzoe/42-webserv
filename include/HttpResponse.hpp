/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/20 17:39:57 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/20 17:54:58 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>

namespace HttpResponse 
{
	std::string build(int code, const std::string &contentType, const std::string &body,
		std::string additionalHeader = "");
};

#endif