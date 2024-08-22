/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vvaudain <vvaudain@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 13:08:26 by vvaudain          #+#    #+#             */
/*   Updated: 2024/08/22 11:57:03 by vvaudain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Webserv.hpp"

#define MAX_EV 4096


int main()
{
	Server server;

	//parse the config file and set the server ports through the vector
	std::vector<int> ports;
	ports.push_back(8080);
	ports.push_back(8081);
	
	server.StartServer(ports);

	server.SetResponse("HTTP/1.1 200 OK\n");
	server.SetResponse("Content-Type: text/html\r\n");
	server.SetResponse("Content-Length: 13\n\n");
	server.SetResponse("Hello World !\r\n\r\n");
	
	
	// signal(SIGINT, SIG_DFL);

	
	
	server.CloseServer();
	return (0);
}