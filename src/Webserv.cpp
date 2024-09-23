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
bool				run = true;

static void	signal_handler(int signum)
{
	(void)signum;
	std::cout << "\r" << BLUE << "SIGINT received, let's shut down the server" << SET << std::endl;
	for (std::vector<int>::iterator it = servers_fd.begin(); it != servers_fd.end(); it++)
		close(*it);
	run = false;
}

static int	create_socket(int port)
{
	struct sockaddr_in	servaddr;
	int 				sockfd;
	int 				optval = 1;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		// std::cerr << "error: socket()" << std::endl;
		perror("socket");
		exit(1);
	}
	if (setsockopt(sockfd, SOL_SOCKET,  SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(int)) < 0)
	{
		perror("setsockopt");
		// std::cerr << "error: setsockopt() failed" << std::endl;
		close(sockfd);
		exit(1);
	}

	// non blocking socket
	if(fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0)
	{
		// std::cerr << "could not set socket to be non blocking" << std::endl;
		perror("fcntl");
		close(sockfd);
		exit(1);
	}
	std::memset((char*)&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(port);
	if (bind(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0)
	{
		std::cerr << "error: bind() failed" << std::endl;
		close(sockfd);
		exit(1);
	}
	if (listen(sockfd, 100) < 0)
	{
		std::cerr << "error: listen() failed" << std::endl;
		close(sockfd);
		exit(1);
	}
	return sockfd;
}

static int	waitingForConnection(struct pollfd *fds, std::vector<int> ports)
{
	const std::string	wait[] = {"⠋", "⠙", "⠸", "⠴", "⠦", "⠇"};
	int 				status = 0;
	int 				n = 0;
	int					timeout = 200;

	std::cout << "Available ports: ";
	for (std::vector<int>::iterator it = ports.begin(); it != ports.end(); it++)
		std::cout << *it << " ";
	std::cout << std::endl;
	while (status == 0)
	{
		std::cout 
			<< "\r" << GREEN 
			<< "Waiting for connection "<< wait[(n++ % 6)] 
			<< SET << "\r" << std::flush;
		if ((status = poll(fds, servers_fd.size(), timeout)) < 0)
		{
			// we could use errno because it's not after read or write (cf subject)
			return -1;
		}
		
	}
	return status;
}

void	webserv(Configuration &config)
{
	std::vector<ServerBlock>	serverBlocks = config.getServerBlocks();
	// std::map<int, Request>		requests;
	struct pollfd				fds[1000];
	char 						buf[1024];
	int							i = 0;

	//		  fd
	std::map<int, Request>		requests;//what we receive from the client, the data we receive
	//		  								check if this data is ok (see in the Request.cpp)
	std::map<int, Response>		responses;// let's parse the request and generate a response
	// this is here we use the configuration


	signal(SIGINT, signal_handler);

	for (std::size_t i = 0; i < serverBlocks.size(); i++)
		servers_fd.push_back(create_socket(serverBlocks[i].port));
	
	for (std::vector<int>::iterator it = servers_fd.begin(); it != servers_fd.end(); it++)
	{
		fds[i].fd = *it; // set descriptor fd to to listening socket
		fds[i].events = 0; // Clear the bit array
		fds[i].events = fds[i].events | POLLIN | POLLOUT | POLLHUP;
		i++;
	}

	/*************************************************************/
	/*                        Server Loop                        */
	/*************************************************************/
	int status = 0;
	int new_socket = 0;
	int count = 1;

	while (run)
	{
		status = waitingForConnection(fds, config.getPorts());

		if (status == -1)
			break;
		
		// Verify which server has a new connection
		for (int j = 0; j < (int)servers_fd.size(); j++)
		{
			if (fds[j].revents & POLLIN)// If the server has a new connection ready
			{
				std::cout << "\r" << "Client connected on server: " << fds[j].fd << std::endl;
				// Accept the connection
				if ((new_socket = accept(fds[j].fd, NULL, NULL)) < 0)
				{
					perror("ACCEPT ERROR");
					continue;
				}
				int ret = 0;
				// Receive the request
				std::cout << "coucou" << std::endl;
				ret = recv(new_socket, &buf, 1023, 0);
				// ret = read(new_socket, &buf, 1023);
				// std::cout << "coucou2" << std::endl;
				if (ret < 0)
				{
					close(new_socket);
					perror("RECV ERROR");
					continue;
				}
				buf[ret] = '\0';
				// std::cout << buf << std::endl;
				std::cout << "requests count : " << count << std::endl;
				count++;
				
				requests[new_socket] = Request();
				requests[new_socket].setRequestState(RECEIVED);
				
				std::stringstream ss;
				ss.write(buf, ret);
				requests[new_socket].parseRequest(ss);

				Response resp = Response(requests[new_socket]);
				
				std::string generatedResp = resp.getResponse(config);
				printRequest(requests[new_socket]);
				// send the response
				if(send(new_socket, generatedResp.c_str(), generatedResp.size(), 0) < 0)
				{
					perror("SEND ERROR");
					continue;
				}
				requests[new_socket].setRequestState(PROCESSED);
			}
			if (fds[j].revents & POLLOUT)
			{
				if (requests[new_socket].getParsingState() == PARSING_DONE)
				{
					// printRequest(requests[new_socket]);
					Response resp = Response(requests[new_socket]);
					std::string generatedResp = resp.getResponse(config);
					if(send(new_socket, generatedResp.c_str(), generatedResp.size(), 0) < 0)
					{
						perror("SEND ERROR");
						continue;
					}
					requests[new_socket].setRequestState(PROCESSED);
					close(new_socket);
				}
			}
			if (fds[j].revents & POLLHUP)
			{
				close(fds[j].fd);
				requests[fds[j].fd].clearRequest(); // check if it's required
				requests.erase(fds[j].fd);
				fds[j].fd = -1;
				fds[j].events = 0;
				fds[j].revents = 0;
				continue;
			}
		}
	}
	for (std::vector<int>::iterator it = servers_fd.begin(); it != servers_fd.end(); it++)
		close(*it);
}
