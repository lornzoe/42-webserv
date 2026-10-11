/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileDescriptor.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ypua <ypua@student.42.singapore.sg>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/21 19:09:44 by ypua              #+#    #+#             */
/*   Updated: 2026/10/11 18:28:37 by ypua             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FileDescriptor.hpp"

FileDescriptor::FileDescriptor(int fd)
	: fd_(fd)
{
}

FileDescriptor::FileDescriptor(std::string filename)
{

	fd_ = open(filename.c_str(), O_RDONLY);
}

FileDescriptor::~FileDescriptor()
{
	closeFd();
}

int FileDescriptor::get() const
{
	return fd_;
}

void FileDescriptor::closeFd()
{
	if (fd_ != -1)
	{
		close(fd_);
		fd_ = -1;
	}
}

void FileDescriptor::resetFd(int fd)
{
	if (fd_ != fd)
	{
		closeFd();
		fd_ = fd;
	}
}
