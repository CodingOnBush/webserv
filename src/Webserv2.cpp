#include "../include/Webserv.hpp"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <poll.h>
#include <vector>
#include <algorithm>

#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define SET "\033[0m"

std::vector<int>	servers_fd;

void signal_handler(int signum)
{
	std::cout << "\r" << BLUE << "SIGINT received, let's shut down the server" << SET << std::endl;
	for (std::vector<int>::iterator it = servers_fd.begin(); it != servers_fd.end(); it++)
		close(*it);
	exit(1);
}

int create_socket(int port)
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
	servaddr.sin_port = htons(port);

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

int waitingForConnection(struct pollfd *fds, int nb_server, int timeout)
{
	int status = 0;
	int n = 0;
	std::string  wait[] = {"⠋", "⠙", "⠸", "⠴", "⠦", "⠇"}; // array of frame

	while (status == 0)
	{
		// std::cout << "\r" << wait[(n++ % 6)] << GREEN << " waiting for connection" << SET << std::flush;
		std::cout << "Waiting for connection" << std::endl;
		// Verify if a new connection is available
		if ((status = poll(fds, nb_server, timeout)) < 0)
		{
			// std::cerr << "error: poll() failed";
			perror("poll error");
			return -1;
		}
	}
	return status;
}

void	webserv(Configuration &config)
{
	std::map<int, Request>	requests;
	struct pollfd			fds[1000];
	char 					buf[1024];

	signal(SIGINT, signal_handler);

	/***********************************************************************/
	/* Creating the different socket listening on all the port provided   */
	/**********************************************************************/

	std::vector<ServerBlock> serverBlocks = config.getServerBlocks();
	for (std::size_t i = 0; i < serverBlocks.size(); i++)
		servers_fd.push_back(create_socket(serverBlocks[i].port));
	

	/*************************************************************/
	/*   Init struct poll fd with previously created sockets     */
	/*************************************************************/
	int i = 0;
	for (std::vector<int>::iterator it = servers_fd.begin(); it != servers_fd.end(); it++)
	{
		fds[i].fd = *it; // set descriptor fd to to listening socket
		fds[i].events = 0; // Clear the bit array
		fds[i].events = fds[i].events | POLLIN | POLLOUT | POLLRDHUP;
		i++;
	}

	/*************************************************************/
	/*                        Server Loop                        */
	/*************************************************************/
	int timeout = 500; // set timeout to 0.5 sec
	int status = 0;
	int new_socket = 0;
	int nb_server = servers_fd.size();
	int count = 1;

	while (true)
	{
		status = waitingForConnection(fds, nb_server, timeout);

		if (status == -1)
			break;
		
		// Verify which server has a new connection
		for (int j = 0; j < nb_server; j++)
		{
			if ((fds[j].revents & POLLIN) == POLLIN)// If the server has a new connection ready
			{
				std::cout << "\r" << "Client connected on server: " << fds[j].fd << std::endl;
				// Accept the connection
				if ((new_socket = accept(fds[j].fd, NULL, NULL)) < 0)
					continue;
				int ret = 0;

				// Receive the request
				if ((ret = recv(new_socket, &buf, 1023, 0)) < 0)
					continue;
				else
				{
					buf[ret] = '\0';
					std::cout << buf << std::endl;
					std::cout << "buf received : " << count << std::endl;
					count++;
					
					// Request	recvReq = Request();
					requests[new_socket] = Request();

					requests[new_socket].setRequestState(RECEIVED);
					std::stringstream ss;
					ss.write(buf, ret);
					requests[new_socket].parseRequest(ss);

					Response resp = Response(requests[new_socket]);
					
					std::string generatedResp = resp.getResponse(config);
				
					// send the response
					if(send(new_socket, generatedResp.c_str(), generatedResp.size(), 0) < 0) {
						continue;
					}
					
					requests[new_socket].setRequestState(PROCESSED);
				}

				
				// close the socket
				// close(new_socket);
			}
		}
	}
}
