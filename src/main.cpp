#include <iostream>

// int main(int argc, char **argv)
// {
// 	(void)argv;
// 	if (argc == 2)
// 	{
// 		std::cout << "Staring webserv!" << std::endl;
// 		return (0);
// 	}
// 	std::cout << "Invalid input! Usage: ./webserv [configuration file]" << std::endl;
// 	return 0;
// }

// /**========================================================================
//  **                           MINI SERVER
//  *?  The goal of this program is to create a program that wait for a request
//  *?  on a port and response always the same message.
//  *
//  **  USAGE: ./mini_serv [PORT]
//  *@param [PORT] int
//  *
//  *@functions :
//  *    socket
//  *    fcntl
//  *    bind
//  *    listen
//  *    accept
//  *    recv
//  *    send
//  *    close
//  *
//  * @behavior :
//  *  If a syscall fail, it will print the syscall involved and exit the program.
//  *  Else, it will display on STDOUT the FULL response in ASCII that you recieved.
//  *  Your server must response correctly to localhost:[PORT] by printing Hello World !
//  *========================================================================**/

// #include <sys/socket.h>
// #include <arpa/inet.h>
// #include <iostream>
// #include <cstring>
// #include <stdlib.h>
// #include <fcntl.h>
// #include <unistd.h>
// #include <cerrno> 

// int main(int ac, char **av)
// {
	
// 	struct sockaddr_in servaddr;
// 	std::string response;
// 	response += "HTTP/1.1 200 OK\n";
// 	response += "Content-Length: 13\n\n";
// 	response += "Hello World !\r\n\r\n";
// 	int server_fd;
// 	int new_socket = 0;
// 	int len;

// 	if (ac != 2)
// 	{
// 		std::cerr << "USAGE: ./mini_serv [PORT]" << std::endl;
// 		exit(1);
// 	}

// /**========================================================================
//  * ?                         CREATE SOCKET
//  *========================================================================**/
// 	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
// 	{
// 		std::cerr << "error: socket()" << std::endl;
// 		exit(1);
// 	}

// /**========================================================================
//  * ?                         TURN SOCKET INTO NON BLOCKING
//  *========================================================================**/
// 	if(fcntl(server_fd, F_SETFL, O_NONBLOCK) < 0)
// 	{
// 		std::cerr << "could not set socket to be non blocking" << std::endl;
// 		exit(1);
// 	}

// /**========================================================================
//  * ?                         FILL SOCKADDR STRUCT
//  *========================================================================**/
// 	std::memset((char*)&servaddr, 0, sizeof(servaddr));
// 	servaddr.sin_family = AF_INET;
// 	servaddr.sin_addr.s_addr = INADDR_ANY;
// 	servaddr.sin_port = htons(std::atoi(av[1]));

// /**========================================================================
//  * ?                         IDENTIFY SOCKET
//  *========================================================================**/
// 	if (bind(server_fd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0)
// 	{
// 		std::cerr << "error: bind()" << std::endl;
// 		exit(1);
// 	}

// /**========================================================================
//  * ?                         LISTEN SOCKET
//  *========================================================================**/
// 	if (listen(server_fd, 100) < 0)
// 	{
// 		std::cerr << "error: listen()" << std::endl;
// 		exit(1);
// 	}

// /**========================================================================
//  * *                                SERVER LOOP
//  * ? ACCEPT: CREATE NEW SOCKET
//  * ? RECV: RECEIVE CLIENT REQUEST
//  * ? SEND: ANSWER TO CLIENT REQUEST
//  * ? CLOSE: CLOSE THE SOCKET
//  *========================================================================**/
// 	while (true)
// 	{
// 		if ((new_socket = accept(server_fd, NULL, NULL)) < 0) {
// 			if (errno == EWOULDBLOCK) {
// 				std::cerr << "No pending connections; sleeping for one second." << std::endl;
// 				sleep(5);
// 			}
// 			else {
// 				std::cerr << "error: accept()" << std::endl;
// 				exit(1);
// 			}
// 			// std::cerr << "error: accept()" << std::endl;
// 			// exit(1);
// 		}
// 		else {
// 			char buffer[1024];

// 			if((len = recv(new_socket, buffer, sizeof buffer - 1, 0)) < 0){
// 				std::cerr << "error: recv()" << std::endl;
// 				exit(1);
// 			}
// 			buffer[len] = '\0';
// 			std::cout << buffer << std::endl;

// 			if(send(new_socket, response.c_str(), response.size(), 0) < 0) {
// 				std::cerr << "error: send()" << std::endl;
// 				exit(1);
// 			}
// 		}
// 		close(new_socket);
// 	}
// 	return (0);
// }

/**========================================================================
**                           EPOLL
*?  The goal of this Proof Of Concept is to reach the pros of asynchronous
*?  programming.
*?  Epoll will give the power to check if a socket is ready to read in it.
*
**  USAGE: ./poll [PORT]
*@param [PORT] int
*
*@functions :
*    socket
*    setsockopt
*    fcntl
*    bind
*    listen
*    epoll
*    accept
*    send
*    close
*
*@behavion
*  This is simple, a program that print ""No pending connections;"
*  every seconds. But when a client is trying to connect to your program,
*  it print "*Client connected on server: server_fd*" on stdout.
*========================================================================**/

#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <sys/epoll.h>

#include <vector>
#include <algorithm>
#include "../include/Server.hpp"

# define GREEN "\033[0;32m"
# define RED "\033[0;31m"
# define YELLOW "\033[0;33m"
# define SET "\033[0m"

#define MAX_EV 4096

// vector to store the servers socket descriptor
// typedef std::vector<int> fd_vector;
// fd_vector servers_fd;

void signal_handler(int signum, Server &server) //I can't do that in terms of syntax no?
{
	std::cout << "\r" << "(" << signum << ") Serveur ending...       " << std::endl;
	// for (fd_vector::iterator it = servers_fd.begin(); it != servers_fd.end(); it++)
	// {
	// 	close(*it);
	// 	epoll_ctl(epfd, EPOLL_CTL_DEL, *it, NULL);
	// }
	close(server->GetServerFd());
	epoll_ctl(server.GetEpollFd(), EPOLL_CTL_DEL, server->GetServerFd(), NULL);
	close(server.GetEpollFd());

	exit(1);
}

int create_socket(char *port)
{
	int listen_fd;
	int on = 1;
	struct sockaddr_in servaddr;

	/*************************************************************/
	/*                      Create socket                        */
	/*************************************************************/
	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		std::cerr << "error: socket()" << std::endl;
		exit(1);
	}

	/*************************************************************/
	/*       Allow socket descriptor to be reuseable             */
	/*************************************************************/
	if (setsockopt(listen_fd, SOL_SOCKET,  SO_REUSEADDR | SO_REUSEPORT, &on, sizeof(int)) < 0)
	{
		std::cerr << "error: setsockopt() failed" << std::endl;
		close(listen_fd);
		exit(1);
	}

	/*************************************************************/
	/* Set socket to be nonblocking. All of the sockets for      */
	/* the incoming connections will also be nonblocking since   */
	/* they will inherit that state from the listening socket.   */
	/*************************************************************/
	if(fcntl(listen_fd, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "could not set socket to be non blocking" << std::endl;
		exit(1);
	}

	/*************************************************************/
	/*               Fill the sock_addr_in struct                */
	/*************************************************************/
	std::memset((char*)&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(std::atoi(port));

	/*************************************************************/
	/*                       Bind the socket                     */
	/*************************************************************/
	if (bind(listen_fd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0)
	{
		std::cerr << "error: bind() failed" << std::endl;
		exit(1);
	}

	/*************************************************************/
	/*                       Listen the socket                   */
	/*************************************************************/
	if (listen(listen_fd, 100) < 0)
	{
		std::cerr << "error: listen() failed" << std::endl;
		exit(1);
	}

	return listen_fd;
}

int	fd_is_server(int ready_fd)
{
	for (fd_vector::iterator it = servers_fd.begin(); it != servers_fd.end(); it++)
		if (*it == ready_fd)
			return *it;
	return 0;
}

int main(int ac, char **av)
{
	// Request buffer and response header (simple one)
	Server server;
	char request[1024];
	server.SetResponse("HTTP/1.1 200 OK\n");
	server.SetResponse("Content-Type: text/html\r\n");
	server.SetResponse("Content-Length: 13\n\n");
	server.SetResponse("Hello World !\r\n\r\n");

	// struct epoll_event used in epoll()
	struct epoll_event event;
	struct epoll_event events[MAX_EV];
	signal(SIGINT, signal_handler);

	if (ac < 2)
	{
		std::cerr << "USAGE: ./epoll [PORT]" << std::endl;
		exit(1);
	}

	/***********************************************************************/
	/* Creating the different socket listening on all the port provided   */
	/**********************************************************************/
	int i = 1;
	while (i < ac)
	{
		server.SetServerFd(create_socket(av[i]));
		// servers_fd.push_back(create_socket(av[i]));
		// i++;
	}

	/*************************************************************/
	/*                     Init struct epoll_event               */
	/*************************************************************/
	epfd = epoll_create1(0);
	i = 0;
	for (fd_vector::iterator it = servers_fd.begin(); it != servers_fd.end(); it++)
	{
		event.data.fd = *it;
		event.events = EPOLLIN;
		epoll_ctl(epfd, EPOLL_CTL_ADD, *it, &event);
	}

	// Loading array for a nice UI
	std::string  wait[] = {"⠋", "⠙", "⠸", "⠴", "⠦", "⠇"}; // array of frame
	int n = 0; // current frame

	/*************************************************************/
	/*                        Server Loop                        */
	/*************************************************************/
	int timeout = 200; // set timeout to 0.5 sec
	int nfds = 0;
	int new_socket = 0;

	while (true)
	{
		errno = 0;

		// Verify if a new connection is available
		if ((nfds = epoll_wait(epfd, events, MAX_EV, timeout)) < 0)
		{
			std::cerr << "error: epoll_wait() failed";
			exit(1);
		}
		else if (nfds == 0)
		{
			std::cout << "\r" << wait[(n++ % 6)] << GREEN << " waiting for connection" << SET << std::flush;
		}

		for (int j = 0; j < nfds; j++)
		{
			int server = 0;
			if (events[j].events & EPOLLERR || events[j].events & EPOLLHUP)
			{
				close(events[j].data.fd);
				continue;
			}
			// If the server has a new connection ready
			if ((server = fd_is_server(events[j].data.fd)) > 0)
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
				epoll_ctl(epfd, EPOLL_CTL_ADD, new_socket, &event);
			}
			else if (events[j].events & EPOLLIN)
			{
				int ret = 0;

				// Receive the request
				if ((ret = recv(events[j].data.fd, &request, 1023, 0)) < 0)
				{
					std::cerr << "error: recv() failed" << strerror(errno) << std::endl;
					exit(1);
				}
				else
				{
					request[ret] = '\0';
					std::cout << request << std::endl;
					event.events = EPOLLOUT;
					event.data.fd = events[j].data.fd;
					epoll_ctl(epfd, EPOLL_CTL_MOD, events[j].data.fd, &event);
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
				epoll_ctl(epfd, EPOLL_CTL_MOD, events[j].data.fd, &event);
				break ;
			}
		}
	}
	return (0);
}