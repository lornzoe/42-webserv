/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UploadStoreDirective.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lyanga <lyanga@student.42singapore.sg>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/14 16:14:50 by lyanga            #+#    #+#             */
/*   Updated: 2026/09/14 17:33:26 by lyanga           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "UploadStoreDirective.hpp"

#include <stdexcept>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>

UploadStoreDirective::UploadStoreDirective(TokenisedBlock::const_iterator &cit) : SimpleDirective(cit)
{
    if (args.size() != 2)
        throw std::runtime_error("upload_store: requires 1 argument.");

    // check location of args[1] to see if the directory exists
    // check as path relative to where ./webserv runs, nginx does this too afaik
    // if the directory doesnt exist or is not writable, throw runtime error
    struct stat st;
    if (stat(args[1].c_str(), &st) != 0 || !S_ISDIR(st.st_mode))
        throw std::runtime_error("upload_store: directory does not exist.");
    if (access(args[1].c_str(), W_OK) != 0)
        throw std::runtime_error("upload_store: directory is not writable.");
    // to check: if we should require the directory to be readable for this directive to pass
    
    upload_location = args[1];
}

void UploadStoreDirective::print(int depth) const
{
for (int i = 0; i < depth; i++)
		std::cout << "  ";
	std::cout << "upload_store: "
        << upload_location
        << ";" << std::endl;
}

const std::string &UploadStoreDirective::getPath() const
{
    return upload_location;
}
