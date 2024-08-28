#pragma once
#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <vector>
#include <map>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include "Request.hpp"

#define MAX_CLIENTS 32
#define MAX_EVENTS 4096
#define BUFFER_SIZE 1024

class Server
{

private:
	std::vector<int> sockets_fd;
	struct sockaddr_in servaddr;
	std::string response; // temporary var

public:
	Server();
	~Server();

	std::vector<int> setUpSockets(std::vector<int> ports);
	int createSocket(int socket_fd);
	void setSocketNonBlocking(int socket_fd);
	void bindAndListen(int socket_fd);
	int createEpoll(std::vector<int> ports);
	void addToInterestList(int epoll_fd, epoll_event ev, std::vector<int>::iterator it);
	void readingLoop(int epoll_fd, epoll_event ev, epoll_event *events);
	void startServer(std::vector<int> ports);
	void acceptConnection(int server, int epoll_fd, epoll_event ev);
	void setResponse(std::string response);
	void sendResponse(int fd, epoll_event ev, int epoll_fd, Request req);
	std::string receiveRequest(int fd, epoll_event ev, int epoll_fd);
};
