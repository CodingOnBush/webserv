#include "../include/Webserv.hpp"

std::map<std::pair<std::string, int>, int> socketsToPorts;
std::map<int, std::vector<ServerBlock> > serversToFd;
std::vector<int> listenFds;
std::vector<struct pollfd> pollFdsList;

#define 		NFDS 1000
struct pollfd	g_fds[NFDS];

std::map<int, Request> requests;
std::map<int, Response> responses;

bool run;

void setNonBlocking(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("fcntl() failed");
}

void setOpt(int fd)
{
	int opt = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("setsockopt() failed");
}

int createSocket(ServerBlock serverBlock, struct sockaddr_in servaddr)
{
	int socket_fd;
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0)
		throw std::runtime_error("socket() failed");
	setNonBlocking(socket_fd);
	setOpt(socket_fd);
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(serverBlock.port);
	return socket_fd;
}

void listenToSockets()
{
	for (std::map<int, std::vector<ServerBlock> >::iterator it = serversToFd.begin(); it != serversToFd.end(); it++)
	{
		int socket_fd = it->first;
		std::vector<ServerBlock> serverBlocks = it->second;
		listenFds.push_back(socket_fd);
	}
}

void rmFromPollWatchlist(int fd)
{
	for (std::size_t i = 0; i < pollFdsList.size(); i++)
	{
		if (pollFdsList[i].fd == fd)
		{
			close(fd);
			pollFdsList.erase(pollFdsList.begin() + i);
			break;
		}
	}
}

void setPollWatchlist(int fd)
{
	// struct pollfd pfd = (struct pollfd){fd, POLLIN | POLLOUT, 0};
	// pollFdsList.push_back(pfd);
	// g_fds[fd] = pfd;
	g_fds[fd].fd = fd;
	g_fds[fd].events = POLLIN | POLLOUT | POLLRDHUP;
}

void initiateWebServer(const Configuration &config)
{
	std::vector<ServerBlock> serverBlocks = config.getServerBlocks();
	for (std::size_t i = 0; i < serverBlocks.size(); i++)
	{
		std::pair<std::string, int> ipPort = std::make_pair(serverBlocks[i].host, serverBlocks[i].port);
		if (socketsToPorts.count(ipPort) == 0)
		{
			struct sockaddr_in servaddr;
			servaddr.sin_family = AF_INET;
			servaddr.sin_addr.s_addr = INADDR_ANY;
			servaddr.sin_port = htons(serverBlocks[i].port);
			int socket_fd = createSocket(serverBlocks[i], servaddr);
			setNonBlocking(socket_fd);
			if (bind(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
				throw std::runtime_error("bind() failed");
			if (listen(socket_fd, MAX_CLIENTS) < 0)
				throw std::runtime_error("listen() failed");
			socketsToPorts[ipPort] = socket_fd;
			serversToFd[socket_fd].push_back(serverBlocks[i]);
		}
		else
		{
			int socket_fd = socketsToPorts[ipPort];
			serversToFd[socket_fd].push_back(serverBlocks[i]);
		}
	}
	listenToSockets();
	for (std::size_t i = 0; i < listenFds.size(); i++)
	{
		setPollWatchlist(listenFds[i]);
	}
}

void acceptConnection(int fd)
{
	struct sockaddr_in cli;
	socklen_t len = sizeof(cli);
	int new_connection = accept(fd, (struct sockaddr *)&cli, &len);
	if (new_connection < 0)
		throw std::runtime_error("accept() failed");
	setNonBlocking(new_connection);
	setOpt(new_connection);
	setPollWatchlist(new_connection);
	serversToFd[new_connection] = serversToFd[fd];
	requests[new_connection] = Request();
}

void receiveRequest(int fd)
{
	char buffer[BUFFER_SIZE];
	std::string fullRequest;
	int ret_total = 0;

	std::size_t return_value = recv(fd, buffer, BUFFER_SIZE, 0);
	if (return_value == -1)
		return;
	
	Request &req = requests[fd];
	if (return_value == 0)
	{
		req.setRequestState(RECEIVED);
		return;
	}
	std::stringstream ss;
	ss.write(buffer, return_value);
	req.parseRequest(ss);
}

void sendResponse(int fd, Configuration &config)
{
	printRequest(requests[fd]);
	Response resp(requests[fd]);
	responses[fd] = resp;
	std::string generatedResponse = responses[fd].getResponse(config);
	int bytes_sent = send(fd, generatedResponse.c_str(), generatedResponse.size(), 0);
	requests[fd].setRequestState(PROCESSED);
}

int findCount(int fd)
{
	int count = 0;
	for (std::size_t i = 0; i < listenFds.size(); i++)
	{
		if (listenFds[i] == fd)
			count++;
	}
	return count;
}

void runWebserver(Configuration &config)
{
	const int	TIMEOUT = 1000;
	int 		count = 0;
	int 		ret;
	int			j;
	int 		fd;


	while (1)
	{
		ret = poll(&g_fds[0], NFDS, TIMEOUT);
		if (ret < 0 )
		{
			perror("PERROR ");
			break;
		}
		if (ret == 0)
			std::cout << "Waiting for connection" << std::endl;
	
		j = 0;
		for (std::size_t i = 0; i < NFDS && j < ret; i++)
		{
			fd = g_fds[i].fd;

			/* If this field is negative, then the corresponding 
			events field is ignored and the revents field returns zero. */
			if (g_fds[i].fd == -1 || g_fds[i].revents == 0)
				continue;
			
			j++;
			if (g_fds[i].revents & POLLRDHUP)
			{
				std::cout << "POLLRDHUP" << std::endl;
				close(g_fds[i].fd);
				requests[g_fds[i].fd].clearRequest(); // check if it's required
				requests.erase(g_fds[i].fd);
				g_fds[i].fd = -1;
				g_fds[i].events = 0;
				g_fds[i].revents = 0;
				continue;
			}
			if (g_fds[i].revents & POLLIN)
			{
				if (findCount(g_fds[i].fd) == 1)
					acceptConnection(g_fds[i].fd);
				else
				{
					std::cout << "count : " << count << std::endl;
					count++;
					receiveRequest(g_fds[i].fd);
				}
			}
			if (g_fds[i].revents & POLLOUT)
			{
				if (requests[g_fds[i].fd].getParsingState() == PARSING_DONE)
					sendResponse(g_fds[i].fd, config);
			}
			// if (requests[g_fds[i].fd].getRequestState() == PROCESSED)
			// {
			// 	// rmFromPollWatchlist(g_fds[i].fd);
			// 	// serversToFd.erase(g_fds[i].fd);
				
			// 	// responses[g_fds[i].fd].clear();
			// 	// responses.erase(g_fds[i].fd);
			// 	// close(g_fds[i].fd);
			// 	// g_fds[i].fd = -1;
			// }
		}
	}
	std::cout << "COUCUOCOCOCOUCUOCOUOUCOUCUCUO" << std::endl;

}


void	closeSockets()
{
	for (std::map<int, std::vector<ServerBlock> >::iterator it = serversToFd.begin(); it != serversToFd.end(); it++)
	{
		close(it->first);
	}
}
