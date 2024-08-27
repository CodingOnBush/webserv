/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vvaudain <vvaudain@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/20 12:06:19 by vvaudain          #+#    #+#             */
/*   Updated: 2024/08/27 12:25:19 by vvaudain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <vector>
#include <map>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#define EXIT_FAILURE 1
#define MAX_CLIENTS 32
#define MAX_EVENTS 4096
#define BUFFER_SIZE 1024
#define yes 1
#define no 0

class Server
{

private:
	std::vector<int>	sockets_fd;
	struct sockaddr_in servaddr;
	std::string			response;
	// Request				request; //containing the buffer

public:
	Server();
	~Server();

	std::vector<int> SetUpSockets(std::vector<int> ports);
	int		CreateSocket(int socket_fd);
	void	SetSocketNonBlocking(int socket_fd);
	void	BindAndListen(int socket_fd);
	int		CreateEpoll(std::vector<int> ports);
	void	AddToInterestList(int epoll_fd, epoll_event ev, std::vector<int>::iterator it);
	void	ReadingLoop(int epoll_fd, epoll_event ev, epoll_event *events);
	void	StartServer(std::vector<int> ports);
	void	AcceptConnection(int server, int epoll_fd, epoll_event ev);
	void	SetResponse(std::string response);
	void	SendResponse(int fd, epoll_event ev, int epoll_fd);
	void	ReceiveRequest(int fd, epoll_event ev, int epoll_fd);
	void	ErrorAndExit(std::string error);
};

#endif