/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vvaudain <vvaudain@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/20 15:17:48 by vvaudain          #+#    #+#             */
/*   Updated: 2024/08/26 14:17:05 by vvaudain         ###   ########.fr       */
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
			ErrorAndExit("fcntl failed");
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
	int new_connection;

	this->SetUpSockets(ports);
	epoll_fd = epoll_create(MAX_CLIENTS + nb_ports);
	if (epoll_fd == -1)
	{
		ErrorAndExit("epoll_create failed");
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
	epoll_event events_buf[MAX_EVENTS];
	while (1)
	{
		// Wait for events
		int nfds = epoll_wait(epoll_fd, events_buf, MAX_EVENTS, -1);
		std::cout << "Server started" << std::endl;
		if (nfds == -1)
		{
			ErrorAndExit("epoll_wait failed");
		}
		// Handle all events
		for (int i = 0; i < nfds; i++)
		{
			for (int j = 0; j < nb_ports; j++)
			{
				if (events_buf[i].data.fd == this->portToSocketMap[ports[j]])
				{
					int new_socket = accept(this->portToSocketMap[ports[j]], (struct sockaddr *)&servaddr, (socklen_t *)&servaddr);
					if (new_socket < 0)
					{
						ErrorAndExit("accept failed");
					}
					if (fcntl(new_socket, F_SETFL, O_NONBLOCK) < 0) //setting the socket to non-blocking
					{
						ErrorAndExit("fcntl failed");
					}
					ev.events = EPOLLIN | EPOLLET; // Here we set the event to be edge-triggered which means that we will be notified only once when the event occurs
					ev.data.fd = new_socket;
					if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &ev) == -1)
					{
						ErrorAndExit("epoll_ctl: new_socket");
					}
					new_connection = yes;
				}
				else
					new_connection = no;
			}
			if (new_connection == no)
			{
				char buffer[BUFFER_SIZE] = {0};
				int client_fd = events_buf[i].data.fd;
				int bytes_read;
				std::string request;
				while ((bytes_read = read(client_fd, buffer, BUFFER_SIZE)) > 0)
				{
					request.append(buffer, bytes_read);
					memset(buffer, 0, BUFFER_SIZE);
				}
				if (bytes_read == 0)
				{
					close(client_fd);
					break;
				}
				else if (bytes_read == -1)
				{
					ErrorAndExit("read failed");
				}
				std::cout << request << std::endl;
			}
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

void Server::ErrorAndExit(std::string error)
{
	std::cerr << error << std::endl;
	exit(EXIT_FAILURE);
}