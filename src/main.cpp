#include <fcntl.h>// fcntl()
#include <unistd.h>// close() fcntl()
#include <iostream>//everything that start with std::
#include <arpa/inet.h>//htons(), htonl()
#include <cstring>//std::memset()
#include <stdio.h>//perror()
#include <signal.h>//signal()

#include <sys/socket.h>
/*
socket() - create an endpoint for communication
int socket(int domain, int type, int protocol);
	- The domain argument specifies a communication 
	domain; this selects the protocol family which will 
	be used for communication. We will use AF_INET : IPv4 Internet protocols
	- The type argument specifies the communication semantics.
	We will use SOCK_STREAM because we want a reliable 
	two-way connection-based byte stream from a TCP socket.
	- The protocol specifies a particular protocol to be used.
	Normally only a single protocol exists to support a 
	particular socket type within a given protocol 
	family, in which case protocol can be specified as 0.

setsockopt() - set options on sockets
int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);

*/



// netstat -an | grep 8080

bool	running = true;

static void	sigintHandler(int sig)
{
	(void)sig;
	running = false;
}

static struct sockaddr_in	createSockAddr(int port)
{
	struct sockaddr_in	address;

	std::memset((char *)&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port);
	return (address);
}

static int	createSocket(int port)
{
	struct sockaddr_in	address = createSockAddr(port);
	int 				optname = SO_REUSEADDR | SO_REUSEPORT;
	int					sockfd = socket(AF_INET, SOCK_STREAM, 0);
	int 				on = 1;

	if (sockfd < 0)
		return (perror("socket() failed"), -1);
	if (setsockopt(sockfd, SOL_SOCKET,  optname, &on, sizeof(int)) < 0)
		return (perror("setsockopt()"), close(sockfd), -1);
	if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0)
		return (perror("fcntl()"), close(sockfd), -1);
	if ((bind(sockfd, (struct sockaddr *) &address, sizeof(address))) < 0)
		return (perror("bind()"), close(sockfd), -1);
	if (listen(sockfd, 3) < 0)
		return (perror("listen()"), close(sockfd), -1);
	return (sockfd);
}

int	main(void)
{
	int	port = 8080;
	int	sockfd = createSocket(port);

	signal(SIGINT, sigintHandler);
	if (sockfd < 0)
		return (1);
	while (running)
	{
		int	newsockfd = accept(sockfd, NULL, NULL);
		if (newsockfd < 0)
			continue ;
		std::cout << "new connection" << std::endl;
		close(newsockfd);
	}
	close(sockfd);
	return (0);
}
