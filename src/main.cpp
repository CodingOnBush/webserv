/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atonkopi <atonkopi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 13:08:26 by vvaudain          #+#    #+#             */
/*   Updated: 2024/08/27 13:38:43 by atonkopi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include "../include/Request.hpp"

#define MAX_EV 4096

// int main()
// {
// 	// signal(SIGINT, SIG_DFL);
// 	// parse the config file and set the server ports through the vector
// 	Server server;

// 	std::vector<int> ports;
// 	ports.push_back(8080);
// 	ports.push_back(8081);

// 	server.StartServer(ports);

// 	server.SetResponse("HTTP/1.1 200 OK\n");
// 	server.SetResponse("Content-Type: text/html\r\n");
// 	server.SetResponse("Content-Length: 13\n\n");
// 	server.SetResponse("Hello World !\r\n\r\n");

// 	// server.CloseServer();
// 	return (0);
// }

int main(int ac, char **av)
{
	(void)ac;
	(void)av;
	Server server;
	
	std::vector<int> ports;
	ports.push_back(8080);
	ports.push_back(8081);
	ports.push_back(8082);

	server.SetResponse("HTTP/1.1 200 OK\n");
	server.SetResponse("Content-Type: text/html\r\n");
	server.SetResponse("Content-Length: 13\n\n");
	server.SetResponse("Hello World !\r\n\r\n");
	
	server.StartServer(ports);
    return 0;
}

