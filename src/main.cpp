/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atonkopi <atonkopi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 13:08:26 by vvaudain          #+#    #+#             */
/*   Updated: 2024/08/27 14:57:36 by atonkopi         ###   ########.fr       */
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


int main(int ac, char **av)
{
	(void)ac;
	(void)av;
	Server server;
	
	std::vector<int> ports;
	ports.push_back(8080);
	ports.push_back(8081);
	ports.push_back(8082);

	server.setResponse("HTTP/1.1 200 OK\n");
	server.setResponse("Content-Type: text/html\r\n");
	server.setResponse("Content-Length: 13\n\n");
	server.setResponse("Hello World !\r\n\r\n");
	
	server.startServer(ports);
    return 0;
}

