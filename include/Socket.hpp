/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vvaudain <vvaudain@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 11:52:49 by vvaudain          #+#    #+#             */
/*   Updated: 2024/08/21 13:39:06 by vvaudain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
# define SOCKET_HPP

#include "Webserv.hpp"
 #include <sys/socket.h>

class Socket {

	private:
		int fd;
		int domain;
		int type;
		int protocol;
		struct sockaddr_in sockAddr;
		int sockAddrLen;

	public:
		Socket();
		~Socket();
		Socket(const Socket &copy);
		Socket &operator=(const Socket &rhs);

		int getFd(void) const;
};


#endif