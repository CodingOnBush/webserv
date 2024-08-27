/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vvaudain <vvaudain@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 13:08:26 by vvaudain          #+#    #+#             */
/*   Updated: 2024/08/27 12:17:49 by vvaudain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

#define MAX_EV 4096


int main()
{
	// signal(SIGINT, SIG_DFL);
	// parse the config file and set the server ports through the vector
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

