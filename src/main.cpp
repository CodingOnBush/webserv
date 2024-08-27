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

<<<<<<< HEAD
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
	struct sockaddr_in servaddr;
=======
int main(int ac, char **av)
{
	(void)ac;
	(void)av;
	Server server;
	
>>>>>>> main
	std::vector<int> ports;
	std::string response;
	ports.push_back(8080);
	response += "HTTP/1.1 200 OK\n";
	ports.push_back(8081);
<<<<<<< HEAD
	response += "Content-Length: 13\n\n";
	response += "Hello World !\r\n\r\n";
	int server_fd;
	int new_socket = 0;
	int len;

	if (ac != 2)
	{
		std::cerr << "USAGE: ./mini_serv [PORT]" << std::endl;
		exit(1);
	}

	/**========================================================================
	 * ?                         CREATE SOCKET
	 *========================================================================**/
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		std::cerr << "error: socket()" << std::endl;
		exit(1);
	}

	/**========================================================================
	 * ?                         TURN SOCKET INTO NON BLOCKING
	 *========================================================================**/
	if (fcntl(server_fd, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "could not set socket to be non blocking" << std::endl;
		exit(1);
	}

	/**========================================================================
	 * ?                         FILL SOCKADDR STRUCT
	 *========================================================================**/
	std::memset((char *)&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(std::atoi(av[1]));

	/**========================================================================
	 * ?                         IDENTIFY SOCKET
	 *========================================================================**/
	if (bind(server_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		std::cerr << "error: bind()" << std::endl;
		exit(1);
	}

	/**========================================================================
	 * ?                         LISTEN SOCKET
	 *========================================================================**/
	if (listen(server_fd, 100) < 0)
	{
		std::cerr << "error: listen()" << std::endl;
		exit(1);
	}

	/**========================================================================
	 * *                                SERVER LOOP
	 * ? ACCEPT: CREATE NEW SOCKET
	 * ? RECV: RECEIVE CLIENT REQUEST
	 * ? SEND: ANSWER TO CLIENT REQUEST
	 * ? CLOSE: CLOSE THE SOCKET
	 *========================================================================**/
	while (true)
	{
		if ((new_socket = accept(server_fd, NULL, NULL)) < 0)
		{
			if (errno == EWOULDBLOCK)
			{
				std::cerr << "No pending connections; sleeping for one second." << std::endl;
				sleep(5);
			}
			else
			{
				std::cerr << "error: accept()" << std::endl;
				exit(1);
			}
		}
		else
		{
			std::string fullRequest;
			char buffer[2000];
			len = recv(new_socket, buffer, sizeof(buffer), 0);
			if (len > 0)
				fullRequest.append(buffer, len);
			if (len == 0)
			{
				std::cout << "Connection closed by client" << std::endl;
				close(new_socket);
				return 0;
			}
			if (len < 0)
			{
				std::cerr << "recv error: " << strerror(errno) << std::endl;
				break;
				}
			Request req(fullRequest);
			req.printRequest(req);
			if (send(new_socket, response.c_str(), response.size(), 0) < 0)
			{
				std::cerr << "error: send()" << std::endl;
				exit(1);
			}
		}
		close(new_socket);
	}
	return (0);
}
=======
	ports.push_back(8082);

	server.setResponse("HTTP/1.1 200 OK\n");
	server.setResponse("Content-Type: text/html\r\n");
	server.setResponse("Content-Length: 13\n\n");
	server.setResponse("Hello World !\r\n\r\n");
	
	server.startServer(ports);
    return 0;
}

>>>>>>> main
