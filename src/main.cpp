#include "../include/socket.hpp"
#include <signal.h>

// #include <vector>

// typedef std::vector<int> fd_vector;

bool	running = true;

static void	sigintHandler(int sig)
{
	(void)sig;
	running = false;
	std::cout << "SIGINT received" << std::endl;
}



// fd_vector	servers_fd;

int	main(void)
{
	signal(SIGINT, sigintHandler);

	struct sockaddr_in		addr;
	socklen_t				len;
	int						sockfd;
	int						sockfd2;
	int						port = 8080;

	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);
	if ((sockfd = createSocket((const sockaddr *)&addr, sizeof(addr))) < 0)
		return (1);
	if ((sockfd2 = createSocket((const sockaddr *)&addr, sizeof(addr))) < 0)
		return (1);
	while (running)
	{
		std::cout << "waiting for connection" << std::endl;
		int	newsockfd = accept(sockfd, NULL, NULL);
		if (newsockfd < 0)
			continue ;
		std::cout << "new connection" << std::endl;
		close(newsockfd);
	}
	close(sockfd);
	return (0);
}
