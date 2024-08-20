/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vvaudain <vvaudain@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/20 15:17:48 by vvaudain          #+#    #+#             */
/*   Updated: 2024/08/20 15:55:43 by vvaudain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(){
}

//We need a initialization constructor to set the server ports
//from the info gathered in the config file
//there can be multiple ports mentioned in the config file, therefore
//we need to create multiple server sockets in a vector or a map

Server::~Server(){
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