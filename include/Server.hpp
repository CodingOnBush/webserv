/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vvaudain <vvaudain@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/20 12:06:19 by vvaudain          #+#    #+#             */
/*   Updated: 2024/08/20 15:59:49 by vvaudain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include "Socket.hpp"

struct sockaddr_in
{
	short			domain; // e.g. AF_INET
	unsigned short	sin_port; // e.g. htons(3490) - We will need to call htons() to ensure that the port is stored in network byte order.
	unsigned long	server_addr;
	int				addr_len;
};


// typedef std::vector<int> fd_vector;
//perhaps we can create a map to map ports with their sockets fds

class Server{

	private:
		Socket				server_socket;
		Socket				new_socket;
		int					server_port;
		// fd_vector			servers_fd;
		struct sockaddr_in	servaddr;
		// Request				request; //containing the buffer
		std::string			response;
		int					epoll_fd;
	
	public:
		Server();
		~Server();
		Server(const Server &src);
		Server &operator=(const Server &rhs);

		int		StartServer(void);
		void 	CloseServer(void);
		void 	LogError(std::string error);
		void 	ExitWithError(std::string error);
		void	SetResponse(std::string response);
		void	SetServerFd(int fd);

};


#endif