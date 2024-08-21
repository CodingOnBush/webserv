/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vvaudain <vvaudain@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/20 15:17:48 by vvaudain          #+#    #+#             */
/*   Updated: 2024/08/21 14:48:50 by vvaudain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(){
	this->server_socket = new Socket();
}

//We need a initialization constructor to set the server ports
//from the info gathered in the config file
//there can be multiple ports mentioned in the config file, therefore
//we need to create multiple server sockets in a vector or a map

Server::~Server(){
}

int Server::StartServer(void){
	
	int opt = 1;

	setsockopt(server_socket->getFd(), SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
	servaddr.domain = AF_INET;
	servaddr.server_addr = INADDR_ANY;
	servaddr.sin_port = htons(server_port);
	
	bind(server_socket->getFd(), (struct sockaddr *)&servaddr, sizeof(servaddr));
	listen(server_socket->getFd(), 3); //3 is the maximum size of the queue of pending connections
	epoll_fd = epoll_create1(0);
	if (epoll_fd == -1)
		ExitWithError("epoll_create1");
	struct epoll_event ev;
	ev.events[MAX_EVENTS];
}

Server::Server(const Server &copy){
	*this = copy;
}

Server &Server::operator=(const Server &rhs){
	if (this == &rhs)
		return (*this);
	server_socket = rhs.server_socket;
	new_socket = rhs.new_socket;
	server_port = rhs.server_port;
	servaddr = rhs.servaddr;
	response = rhs.response;
	epoll_fd = rhs.epoll_fd;
	return (*this);
}

void Server::SetResponse(std::string response){
	if (response.empty())
		this->response = response;
	else
		this->response += response;
}

void Server::SetServerFd(int fd){
	this->server_socket.fd = fd;
}