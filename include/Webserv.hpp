/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vvaudain <vvaudain@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 11:52:04 by vvaudain          #+#    #+#             */
/*   Updated: 2024/08/22 11:48:30 by vvaudain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <vector>
#include <map>

struct sockaddr_in
{
	short			domain; // e.g. AF_INET
	unsigned short	sin_port; // e.g. htons(3490) - We will need to call htons() to ensure that the port is stored in network byte order.
	unsigned long	server_addr;
	int				addr_len;
};

#endif