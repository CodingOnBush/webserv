/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   OldServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vvaudain <vvaudain@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/20 15:17:48 by vvaudain          #+#    #+#             */
/*   Updated: 2024/08/26 16:19:43 by vvaudain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

// Server::Server()
// {
// }

// std::vector<int> Server::SetUpSockets(std::vector<int> ports)
// {
// 	std::vector<int> servers_fd;
// 	int socket_fd;
// 	int opt = 1;

// 	for (std::vector<int>::iterator it = ports.begin(); it != ports.end(); it++)
// 	{
// 		socket_fd = socket(AF_INET, SOCK_STREAM, 0);
// 		if (socket_fd < 0)
// 		{
// 			ErrorAndExit("socket failed");
// 		}
// 		setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
// 		if (fcntl(socket_fd, F_SETFL, O_NONBLOCK) < 0) //setting the socket to non-blocking
// 		{
// 			ErrorAndExit("fcntl failed");
// 		}
// 		servaddr.sin_family = AF_INET;
// 		servaddr.sin_addr.s_addr = INADDR_ANY;
// 		servaddr.sin_port = htons(*it);
// 		bind(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr));
// 		listen(socket_fd, MAX_CLIENTS); //32 is the maximum size of the queue of pending connections
// 		servers_fd.push_back(socket_fd);
// 	}
// 	return servers_fd;
// }

// std::vector<int> Server::SetUpSockets(std::vector<int> ports) {
    
// 	std::vector<int> servers_fd;
// 	int sfd;
	
// 	for (std::vector<int>::iterator it = ports.begin(); it != ports.end(); ++it) {
//         int port = *it;
//         sfd = socket(AF_INET, SOCK_STREAM, 0);
//         if (sfd < 0) {
//             perror("socket");
//             exit(EXIT_FAILURE);
//         }
// 		servers_fd.push_back(sfd);

//         servaddr.sin_family = AF_INET;
//         servaddr.sin_addr.s_addr = INADDR_ANY;
//         servaddr.sin_port = htons(port);

//         if (bind(sfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
//             perror("bind");
//             close(sfd);
//             exit(EXIT_FAILURE);
//         }

//         if (listen(sfd, MAX_CLIENTS) < 0) {
//             perror("listen");
//             close(sfd);
//             exit(EXIT_FAILURE);
//         }

//         portToSocketMap[port] = sfd;
//     }
// }

// We need a initialization constructor to set the server ports
// from the info gathered in the config file
// there can be multiple ports mentioned in the config file, therefore
// we need to create multiple server sockets in a vector or a map

// Server::~Server()
// {
// }

int	fd_is_server(int ready_fd, std::vector<int> servers_fd)
{
	for (std::vector<int>::iterator it = servers_fd.begin(); it != servers_fd.end(); it++)
		if (*it == ready_fd)
			return *it;
	return 0;
}

void Server::StartServer(std::vector<int> ports)
{
	int nb_ports = static_cast<int>(ports.size());
	std::vector<int> servers_fd;
	// int new_connection;

	servers_fd = this->SetUpSockets(ports);
	epoll_fd = epoll_create(MAX_CLIENTS + nb_ports);
	if (epoll_fd == -1)
	{
		ErrorAndExit("epoll_create failed");
	}
	epoll_event event;
	event.events = EPOLLIN;
	for (std::vector<int>::iterator it = servers_fd.begin(); it != servers_fd.end(); it++)
	{
		event.data.fd = *it;
		event.events = EPOLLIN;
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, *it, &event);
	}
	// for (int i = 0; i < nb_ports; i++)
	// {
	// 	event.data.fd = this->portToSocketMap[ports[i]];
	// 	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, this->portToSocketMap[ports[i]], &event) == -1)
	// 	{
	// 		perror("epoll_ctl: listen_sock");
	// 		exit(EXIT_FAILURE);
	// 	}
	// }
	epoll_event events[MAX_EVENTS];
	// while (1)
	// {
	// 	// Wait for events
	// 	int nfds;
	// 	if ((nfds = epoll_wait(epoll_fd, events_buf, MAX_EVENTS, 200) < 0))
	// 	{
	// 		ErrorAndExit("epoll_wait");
	// 	}
	// 	// Handle all events
	// 	for (int i = 0; i < nfds; i++)
	// 	{
	// 		for (int j = 0; j < nb_ports; j++)
	// 		{
	// 			if (events_buf[i].data.fd == this->portToSocketMap[ports[j]])
	// 			{
	// 				int new_socket = accept(this->portToSocketMap[ports[j]], (struct sockaddr *)&servaddr, (socklen_t *)&servaddr);
	// 				if (new_socket < 0)
	// 				{
	// 					ErrorAndExit("accept failed");
	// 				}
	// 				if (fcntl(new_socket, F_SETFL, O_NONBLOCK) < 0) //setting the socket to non-blocking
	// 				{
	// 					ErrorAndExit("fcntl failed");
	// 				}
	// 				ev.events = EPOLLIN | EPOLLET; // Here we set the event to be edge-triggered which means that we will be notified only once when the event occurs
	// 				ev.data.fd = new_socket;
	// 				if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &ev) == -1)
	// 				{
	// 					ErrorAndExit("epoll_ctl: new_socket");
	// 				}
	// 				new_connection = yes;
	// 			}
	// 			else
	// 				new_connection = no;
	// 		}
	// 		if (new_connection == no)
	// 		{
	// 			char buffer[BUFFER_SIZE] = {0};
	// 			int client_fd = events_buf[i].data.fd;
	// 			int bytes_read;
	// 			std::string request;
	// 			while ((bytes_read = read(client_fd, buffer, BUFFER_SIZE)) > 0)
	// 			{
	// 				request.append(buffer, bytes_read);
	// 				memset(buffer, 0, BUFFER_SIZE);
	// 			}
	// 			if (bytes_read == 0)
	// 			{
	// 				close(client_fd);
	// 				break;
	// 			}
	// 			else if (bytes_read == -1)
	// 			{
	// 				ErrorAndExit("read failed");
	// 			}
	// 			std::cout << request << std::endl;
	// 		}
	// 	}
		
	// }
	int timeout = 200; // set timeout to 0.5 sec
	int nfds = 0;
	int new_socket = 0;
	int i = 0;
	
	while (true)
	{
		errno = 0;

		// Verify if a new connection is available
		if ((nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, timeout)) < 0)
		{
			std::cerr << "error: epoll_wait() failed";
			exit(1);
		}
		for (int j = 0; j < nfds; j++)
		{
			int server = 0;
			if (events[j].events & EPOLLERR || events[j].events & EPOLLHUP)
			{
				close(events[j].data.fd);
				continue;
			}
			std::cout << "server nb" << events[j].data.fd << std::endl;
			// If the server has a new connection ready
			if ((server = fd_is_server(events[j].data.fd, servers_fd)) != 0)
			{
				if ((new_socket = accept(server, NULL, NULL)) < 0)
				{
					if(errno != EWOULDBLOCK)
					{
						std::cerr << "error: accept() failed" << std::endl;
						exit(1);
					}
				}
				std::cout << "\r" << "Client connected on server: " << events[j].data.fd << std::endl;
				fcntl(new_socket, F_SETFL, O_NONBLOCK);
				event.data.fd = new_socket;
				event.events = EPOLLIN;
				epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &event);
			
				std::cout << "EVENT: " << events[j].events << std::endl;
				std::cout << "EVENT: " << EPOLLIN << std::endl;
			}
			else if (events[j].events & EPOLLIN)
			{
				int ret = 0;

				// Receive the request
				std::string request;
				if ((ret = recv(events[j].data.fd, &request, 1023, 0)) < 0)
				{
					std::cerr << "error: recv() failed" << strerror(errno) << std::endl;
					exit(1);
				}
				else
				{
					std::cout << request << std::endl;
					// request[ret] = '\0';
					event.events = EPOLLOUT;
					event.data.fd = events[j].data.fd;
					epoll_ctl(epoll_fd, EPOLL_CTL_MOD, events[j].data.fd, &event);
				}
				break ;
			}
			else if (events[i].events & EPOLLOUT)
			{
				if(send(events[j].data.fd, response.c_str(), response.size(), 0) < 0)
				{
					std::cerr << "error: send() failed" << std::endl;
					exit(1);
				}
				event.events = EPOLLIN;
				event.data.fd = events[j].data.fd;
				// if not keep-alive
				close(events[j].data.fd);
				epoll_ctl(epoll_fd, EPOLL_CTL_MOD, events[j].data.fd, &event);
				break ;
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