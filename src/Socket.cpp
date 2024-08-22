/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vvaudain <vvaudain@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 11:54:57 by vvaudain          #+#    #+#             */
/*   Updated: 2024/08/21 15:12:22 by vvaudain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

Socket::Socket()
{
	this->fd = socket(AF_INET , SOCK_STREAM, 0);
}

Socket::~Socket()
{
}

int Socket::getFd(void) const {
	return this->fd;
}