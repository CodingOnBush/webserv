/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vvaudain <vvaudain@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/20 15:17:48 by vvaudain          #+#    #+#             */
/*   Updated: 2024/08/22 14:30:45 by vvaudain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server()
{
}

void Server::SetUpSockets(std::vector<int> ports)
{

	int socket_fd;
	int opt = 1;

	for (int i = 0; i < static_cast<int>(ports.size()); i++)
	{
		socket_fd = socket(AF_INET, SOCK_STREAM, 0);
		this->portToSocketMap[ports[i]] = socket_fd;
		setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
		servaddr.sin_family = AF_INET;
		// servaddr.sin_addr = INADDR_ANY;
		servaddr.sin_port = htons(this->portToSocketMap[ports[i]]);
		bind(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr));
		listen(socket_fd, 32); //32 is the maximum size of the queue of pending connections
	}
}

// We need a initialization constructor to set the server ports
// from the info gathered in the config file
// there can be multiple ports mentioned in the config file, therefore
// we need to create multiple server sockets in a vector or a map

Server::~Server()
{
}

void Server::StartServer(std::vector<int> ports)
{
	this->SetUpSockets(ports);
	// epoll_fd = epoll_create1(0);
	if (epoll_fd == -1)
	{
		perror("epoll_create1");
		exit(1);
	}
	// struct epoll_event ev;
	// (void)ev;
	// ev.events[MAX_EVENTS];
}

void Server::SetResponse(std::string response)
{
	if (response.empty())
		this->response = response;
	else
		this->response += response;
}
