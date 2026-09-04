/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileDescriptor.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ypua <ypua@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/21 19:09:44 by ypua              #+#    #+#             */
/*   Updated: 2026/07/21 19:31:32 by ypua             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FileDescriptor.hpp"

FileDescriptor::FileDescriptor(std::string filename)
{

	fd_ = open(filename.c_str(), O_RDONLY);
}

FileDescriptor::~FileDescriptor()
{
	if (fd_ != -1)
		close(fd_);
}

int FileDescriptor::get()
{
	return fd_;
}