/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: momrane <momrane@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 13:08:26 by vvaudain          #+#    #+#             */
/*   Updated: 2024/08/26 11:30:34 by momrane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

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

#include "../include/Configuration.hpp"

int	main(int ac, char **av)
{
	if (ac != 2)
	{
		std::cerr << "Usage: ./webserv <config_file>" << std::endl;
		return (1);
	}
	std::cout << "Config file: " << av[1] << std::endl;
	// What's inside the config file?
	try {
		/*
		vic need this :
		std::vector<int> ports;
		*/
		Configuration	config(av[1]);
	}
	catch (std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return (1);
	}
	return (0);
}
