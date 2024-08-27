/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vvaudain <vvaudain@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/26 15:53:08 by vvaudain          #+#    #+#             */
/*   Updated: 2024/08/27 12:33:53 by vvaudain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server()
{
}

Server::~Server()
{
}

int Server::CreateSocket(int socket_fd)
{
	int opt = 1;
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0)
	{
		ErrorAndExit("socket failed");
	}
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
	{
		std::cerr << "setsockopt failed" << std::endl;
		close(socket_fd);
		exit(EXIT_FAILURE);
	}
	return socket_fd;
}

void Server::ErrorAndExit(std::string error)
{
	std::cerr << error << std::endl;
	exit(EXIT_FAILURE);
}

void Server::SetSocketNonBlocking(int socket_fd)
{
	if (fcntl(socket_fd, F_SETFL, O_NONBLOCK) < 0) // setting the socket to non-blocking
	{
		ErrorAndExit("fcntl failed");
	}
}

void Server::BindAndListen(int socket_fd)
{
	if (bind(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		ErrorAndExit("bind() failed");
	}
	if (listen(socket_fd, MAX_CLIENTS) < 0) // 32 is the maximum size of the queue of pending connections
	{
		ErrorAndExit("listen() failed");
	}
}

std::vector<int> Server::SetUpSockets(std::vector<int> ports)
{
	std::vector<int> servers_fd;
	int socket_fd;

	for (std::vector<int>::iterator it = ports.begin(); it != ports.end(); it++)
	{
		socket_fd = CreateSocket(socket_fd);
		SetSocketNonBlocking(socket_fd);
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = INADDR_ANY;
		servaddr.sin_port = htons(*it);
		BindAndListen(socket_fd);
		servers_fd.push_back(socket_fd);
	}
	return servers_fd;
}

int Server::CreateEpoll(std::vector<int> ports)
{
	int epoll_fd = epoll_create(MAX_EVENTS + ports.size());
	if (epoll_fd < 0)
	{
		ErrorAndExit("epoll_create failed");
	}
	return epoll_fd;
}

void Server::AddToInterestList(int epoll_fd, epoll_event ev, std::vector<int>::iterator it)
{
	ev.events = EPOLLIN;
	ev.data.fd = *it;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, *it, &ev) < 0)
	{
		ErrorAndExit("epoll_ctl failed");
	}
}

int fd_is_server(int fd, std::vector<int> sockets_fd)
{
	for (std::vector<int>::iterator it = sockets_fd.begin(); it != sockets_fd.end(); it++)
	{
		if (fd == *it)
			return (*it);
	}
	return 0;
}

void Server::ReadingLoop(int epoll_fd, epoll_event ev, epoll_event *events)
{
	int nb_fds = 0;
	int timeout = 1000;
	int j = 0;

	while (1)
	{
		if ((nb_fds = epoll_wait(epoll_fd, events, MAX_EVENTS, timeout)) < 0)
		{
			ErrorAndExit("epoll_wait() failed");
		}
		else if (nb_fds == 0)
		{
			std::cout << "Waiting for connection" << std::endl;
		}
		for (int i = 0; i < nb_fds; i++)
		{
			int server = 0;
			if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP)
			{
				std::cerr << "epoll error" << std::endl;
				close(events[i].data.fd);
				continue;
			}
			if ((server = fd_is_server(events[i].data.fd, this->sockets_fd)) > 0)
			{
				AcceptConnection(server, epoll_fd, ev);
			}
			else if (events[i].events & EPOLLIN)
			{
				ReceiveRequest(events[i].data.fd, ev, epoll_fd);
				break;
			}
			else if (events[j].events & EPOLLOUT)
			{
				SendResponse(events[j].data.fd, ev, epoll_fd);
				break;
			}
		}
	}
}

void Server::AcceptConnection(int server, int epoll_fd, epoll_event ev)
{
	int new_connection = accept(server, NULL, NULL);
	if (new_connection < 0)
	{
		std::cerr << "accept() failed" << std::endl;
		exit(EXIT_FAILURE);
	}
	std::cout << "New client connection to server accepted" << std::endl;
	SetSocketNonBlocking(new_connection);
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = new_connection;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_connection, &ev) < 0)
	{
		std::cerr << "epoll_ctl failed" << std::endl;
		close(new_connection);
	}
}

void Server::ReceiveRequest(int fd, epoll_event ev, int epoll_fd)
{
	int		return_value = 0;
	char	buffer[BUFFER_SIZE];
				
	return_value = recv(fd, buffer, BUFFER_SIZE - 1, 0);
	if (return_value < 0)
	{
		std::cerr << "recv() failed" << std::endl;
		close(fd);
		exit(EXIT_FAILURE);
	}
	else
	{
		buffer[return_value] = '\0';
		std::cout << "Received request: " << buffer << std::endl;
		ev.events = EPOLLOUT;
		ev.data.fd = fd;
		epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
	}
}

void Server::SendResponse(int fd, epoll_event ev, int epoll_fd)
{
	if (send(fd, response.c_str(), response.size(), 0) < 0)
	{
		ErrorAndExit("send() failed");
	}
	std::cout << "Response sent" << response << std::endl;
	ev.events = EPOLLIN;
	ev.data.fd = fd;
	close(fd);
	epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
}

void Server::StartServer(std::vector<int> ports)
{
	this->sockets_fd = SetUpSockets(ports);
	int epoll_fd = CreateEpoll(ports);
	epoll_event ev;
	epoll_event events[MAX_EVENTS];

	for (std::vector<int>::iterator it = sockets_fd.begin(); it != sockets_fd.end(); it++)
		AddToInterestList(epoll_fd, ev, it);

	ReadingLoop(epoll_fd, ev, events);
}

void Server::SetResponse(std::string response)
{
	if (response.empty())
		this->response = response;
	else
		this->response += response;
}