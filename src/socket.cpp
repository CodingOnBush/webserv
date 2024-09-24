#include "../include/socket.hpp"

struct sockaddr_in	*createSockAddr(int port)
{
	struct sockaddr_in	address;

	std::memset((char *)&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port);
	return (&address);
}

int	createSocket(int port, struct sockaddr_in *address)
{
	int 				optname = SO_REUSEADDR | SO_REUSEPORT;
	int					sockfd = socket(AF_INET, SOCK_STREAM, 0);
	int 				on = 1;

	if (sockfd < 0)
		return (perror("socket() failed"), -1);
	if (setsockopt(sockfd, SOL_SOCKET,  optname, &on, sizeof(int)) < 0)
		return (perror("setsockopt()"), close(sockfd), -1);
	// if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0)
	// 	return (perror("fcntl()"), close(sockfd), -1);
	if ((bind(sockfd, address, sizeof(address))) < 0)
		return (perror("bind()"), close(sockfd), -1);
	if (listen(sockfd, 3) < 0)
		return (perror("listen()"), close(sockfd), -1);
	return (sockfd);
}
