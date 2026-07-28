/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileDescriptor.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ypua <ypua@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/21 19:09:51 by ypua              #+#    #+#             */
/*   Updated: 2026/07/21 19:31:34 by ypua             ###   ########.fr       */
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
	FileDescriptor(std::string filename);
	~FileDescriptor();

	int get();

private:
	int fd_;
};

#endif