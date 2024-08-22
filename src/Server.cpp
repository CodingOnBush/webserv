/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vvaudain <vvaudain@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/20 15:17:48 by vvaudain          #+#    #+#             */
/*   Updated: 2024/08/22 15:37:27 by vvaudain         ###   ########.fr       */
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
		if (fcntl(socket_fd, F_SETFL, O_NONBLOCK) < 0) //setting the socket to non-blocking
		{
			perror("fcntl");
			exit(EXIT_FAILURE);
		}
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = INADDR_ANY;
		servaddr.sin_port = htons(this->portToSocketMap[ports[i]]);
		bind(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr));
		listen(socket_fd, MAX_CLIENTS); //32 is the maximum size of the queue of pending connections
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
	int nb_ports = static_cast<int>(ports.size());

	this->SetUpSockets(ports);
	epoll_fd = epoll_create1(0);
	if (epoll_fd == -1)
	{
		perror("epoll_create1");
		exit(EXIT_FAILURE);
	}
	epoll_event ev;
	ev.events = EPOLLIN;
	for (int i = 0; i < nb_ports; i++)
	{
		ev.data.fd = this->portToSocketMap[ports[i]];
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, this->portToSocketMap[ports[i]], &ev) == -1)
		{
			perror("epoll_ctl: listen_sock");
			exit(EXIT_FAILURE);
		}
	}
}

void Server::SetResponse(std::string response)
{
	if (response.empty())
		this->response = response;
	else
		this->response += response;
}