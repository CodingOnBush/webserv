/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vvaudain <vvaudain@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 11:54:57 by vvaudain          #+#    #+#             */
/*   Updated: 2024/08/21 13:39:21 by vvaudain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

Socket::Socket() : domain(AF_INET), type(SOCK_STREAM), protocol(0)
{
	this->fd = socket(this->domain, this->type, this->protocol);
	
}

Socket::~Socket()
{
}

Socket::Socket(const Socket &copy)
{
	*this = copy;
}

Socket &Socket::operator=(const Socket &rhs)
{
	if (this != &rhs)
	{
		this->fd = rhs.fd;
		this->domain = rhs.domain;
		this->type = rhs.type;
		this->protocol = rhs.protocol;
		this->sockAddr = rhs.sockAddr;
		this->sockAddrLen = rhs.sockAddrLen;
	}
	return *this;
}

int Socket::getFd(void) const {
	return this->fd;
}