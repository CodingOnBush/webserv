#include "../include/socket.hpp"

int	createSocket(const sockaddr *addr, socklen_t len)
{
	int	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	int	optname = SO_REUSEADDR | SO_REUSEPORT;
	/*
	SO_REUSEADDR
		Reuse of local addresses is supported.
	SO_REUSEPORT
		Permits multiple AF_INET or AF_INET6 sockets to 
		be bound to an identical socket address.
	*/
	int	on = 1;

	if (sockfd < 0)
		return (perror("socket() failed"), -1);
	if (setsockopt(sockfd, SOL_SOCKET,  optname, &on, sizeof(int)) < 0)
		return (perror("setsockopt()"), close(sockfd), -1);
	// if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0)
	// 	return (perror("fcntl()"), close(sockfd), -1);
	if ((bind(sockfd, addr, len)) < 0)
		return (perror("bind()"), close(sockfd), -1);
	if (listen(sockfd, 42) < 0)
		return (perror("listen()"), close(sockfd), -1);
	return (sockfd);
}
