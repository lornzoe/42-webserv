/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MimeTypes.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/02 00:00:00 by lyanga            #+#    #+#             */
/*   Updated: 2026/08/02 04:07:46 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MIMETYPES_HPP
#define MIMETYPES_HPP

#include <string>

namespace MimeTypes {

    const std::string &     forExtn(const std::string &extension);
    const std::string &     forPath(const std::string &resource_path);

}

#endif
