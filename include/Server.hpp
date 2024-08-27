/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vvaudain <vvaudain@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/20 12:06:19 by vvaudain          #+#    #+#             */
/*   Updated: 2024/08/22 14:27:19 by vvaudain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <arpa/inet.h>
// #include <sys/epoll.h>
#include <vector>
#include <map>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_EVENTS 4096

class Server
{

private:
	std::map<int, int> portToSocketMap;
	struct sockaddr_in servaddr;
	// Request				request; //containing the buffer
	std::string response;
	int epoll_fd;

public:
	Server();
	~Server();

	void SetUpSockets(std::vector<int> ports);
	void StartServer(std::vector<int> ports);
	void SetResponse(std::string response);
};

#endif
