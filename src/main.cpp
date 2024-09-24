#include "../include/socket.hpp"
#include <signal.h>

#include <vector>

bool	running = true;

static void	sigintHandler(int sig)
{
	(void)sig;
	running = false;
}

typedef std::vector<int> fd_vector;

fd_vector	servers_fd;

int	main(void)
{
	signal(SIGINT, sigintHandler);

	int	port = 8080;
	struct sockaddr_in	address = createSockAddr(port);
	
	int	sockfd = createSocket(port);

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
