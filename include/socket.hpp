#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>
#include <cstring>
#include <stdio.h>
#include <sys/socket.h>

int	createSocket(int port);

#endif