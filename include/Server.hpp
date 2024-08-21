/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vvaudain <vvaudain@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/20 12:06:19 by vvaudain          #+#    #+#             */
/*   Updated: 2024/08/21 14:49:10 by vvaudain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include "Webserv.hpp"
#include "Socket.hpp"

#define MAX_EVENTS 4096

// typedef std::vector<int> fd_vector;
//perhaps we can create a map to map ports with their sockets fds

class Server{

	private:
		Socket				*server_socket;
		Socket				*new_socket;
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