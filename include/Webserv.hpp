#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "Request.hpp"
#include "Response.hpp"
#include "Configuration.hpp"
#include "runServer.hpp"

#include <utility>
#include <algorithm>
#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <vector>
#include <map>
#include <set>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <poll.h>

#define MAX_CLIENTS 32
#define BUFFER_SIZE 1024

#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define SET "\033[0m"

extern int uploadNb;

void webserv(Configuration &config); // main function to run the server

void initiateWebServer(Configuration &config);
void runWebserver(Configuration &config);

#endif // WEBSERV_HPP