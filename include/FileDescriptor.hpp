/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileDescriptor.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ypua <ypua@student.42.singapore.sg>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/21 19:09:51 by ypua              #+#    #+#             */
/*   Updated: 2026/10/11 18:28:35 by ypua             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FILEDESCRIPTOR_H
#define FILEDESCRIPTOR_H

#include <iostream>
#include <fcntl.h>
#include <unistd.h>

class FileDescriptor
{
public:
	FileDescriptor(int fd);
	FileDescriptor(std::string filename);
	~FileDescriptor();

	int get() const;
	void closeFd();
	void resetFd(int fd);

private:
	int fd_;

	FileDescriptor(const FileDescriptor &other);
	FileDescriptor &operator=(const FileDescriptor &other);
};

#endif
