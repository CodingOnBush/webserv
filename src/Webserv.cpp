#include "../include/Webserv.hpp"
#include "Webserv.hpp"

struct Connection
{
	int		fd;
	std::string	host;
	int			port;
	std::time_t	startTime;
};

std::map<int, std::time_t> startTimeForFd;

typedef std::pair<std::string, int> Hostport;


// std::map<std::pair<std::string, int>, int>	hostPort_to_socket_fd;
std::map<int, ServerBlock>	socketFd_to_ServerBlock;
std::map<int, std::time_t>	fdToTimeoutCheck;
std::vector<int>			listenFds;
std::vector<struct pollfd>	pollFdsList;
std::map<int, Request>		requests;
std::map<int, Response> 	responses;
bool						running = true;

static void printPollFdsList(std::vector<struct pollfd> &pollFdsList)
{
	std::cout << "PollFdsList ( " << pollFdsList.size() << " fds):" << std::endl;
	for (std::vector<struct pollfd>::iterator it = pollFdsList.begin(); it != pollFdsList.end(); it++)
	{
		std::cout << "fd: " << it->fd << " events: " << it->events << " revents: " << it->revents << std::endl;
	}
}

static void	printListenFds()
{
	std::cout << "ListenFds (" << listenFds.size() << "):" << std::endl;
	for (std::vector<int>::iterator it = listenFds.begin(); it != listenFds.end(); it++)
	{
		std::cout << *it << std::endl;
	}
}

// static void	printSocketsToPorts()
// {
// 	std::cout << "SocketsToPorts (" << socketsToPorts.size() << "):" << std::endl;
// 	for (std::map<std::pair<std::string, int>, int>::iterator it = socketsToPorts.begin(); it != socketsToPorts.end(); it++)
// 	{
// 		std::cout << it->first.first << ":" << it->first.second << "(host:port) -> socket: " << it->second << std::endl;
// 	}
// }

// static void	printServersToFd()
// {
// 	std::cout << "ServersToFd (" << socketFd_to_ServerBlock.size() << "):" << std::endl;
// 	for (std::map<int, std::vector<ServerBlock> >::iterator it = socketFd_to_ServerBlock.begin(); it != socketFd_to_ServerBlock.end(); it++)
// 	{
// 		ServerBlock	server = it->second[0];
// 		std::cout << "fd: " << it->first << " server port: " << server.port << std::endl;
// 	}
// }

void rmFromPollWatchlist(int fd)
{
	for (std::vector<struct pollfd>::iterator it = pollFdsList.begin(); it != pollFdsList.end(); it++)
	{
		if (it->fd == fd)
		{
			pollFdsList.erase(it);
			break;
		}
	}
}

static int	createServerSocket(int port)
{
	int 				serverSocket;
	struct sockaddr_in	addr;
	int 				opt = 1;

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1)
		return (perror("socket"), -1);
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		return (perror("setsockopt"), close(serverSocket), -1);
	memset((char*)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	if (bind(serverSocket, (struct sockaddr *)&addr, sizeof(addr)) == -1)
		return (perror("bind"), close(serverSocket), -1);
	if (listen(serverSocket, MAX_CLIENTS) == -1)
		return (perror("listen"), close(serverSocket), -1);
	return (serverSocket);
}

static bool	socketAlreadyExists(Hostport hostPort)
{
	int	sizes = socketFd_to_ServerBlock.size();
	for (int i = 0; i < sizes; i++)
	{
		if (socketFd_to_ServerBlock[i].host == hostPort.first && socketFd_to_ServerBlock[i].port == hostPort.second)
			return true;
	}
	return false;
}

static void	printSocketFd_to_ServerBlock()
{
	std::cout << "SocketFd_to_ServerBlock (" << socketFd_to_ServerBlock.size() << "):" << std::endl;
	for (std::map<int, ServerBlock>::iterator it = socketFd_to_ServerBlock.begin(); it != socketFd_to_ServerBlock.end(); it++)
	{
		std::cout << "fd: " << it->first << " host: " << it->second.host << " port: " << it->second.port << std::endl;
	}
}

static void	initServer(Configuration &config)
{
	std::vector<ServerBlock>	servers;
	int							nservers;
	
	servers = config.getServerBlocks();
	nservers = servers.size();
	for (int i = 0; i < nservers; i++)
	{
		ServerBlock	currentServer = servers[i];
		Hostport	hostPort = std::make_pair(currentServer.host, currentServer.port);
		int			server_fd;

		if (socketAlreadyExists(hostPort))
			continue;
		server_fd = createServerSocket(currentServer.port);
		if (server_fd == -1)
			continue;
		socketFd_to_ServerBlock[server_fd] = currentServer;
		listenFds.push_back(server_fd);
	}
	for (std::vector<int>::iterator it = listenFds.begin(); it != listenFds.end(); it++)
	{
		struct pollfd pfd;
		pfd.fd = *it;
		pfd.events = POLLIN | POLLOUT | POLLHUP;
		pollFdsList.push_back(pfd);
	}
	requests.clear();
	responses.clear();
}

void	acceptConnection(int fd)
{
	int	newConnection = accept(fd, NULL, NULL);
	int	opt = 1;
	
	if (newConnection < 0)
	{
		perror("accept");
		return;
	}
	if (fcntl(newConnection, F_SETFL, O_NONBLOCK) < 0)
	{
		close(newConnection);
		perror("fcntl");
		return;
	}
	if (setsockopt(newConnection, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
	{
		close(newConnection);
		perror("setsockopt");
		return;
	}
	struct pollfd	pfd = (struct pollfd){newConnection, POLLIN | POLLOUT | POLLHUP, 0};
	pollFdsList.push_back(pfd);
	// std::cout << "PollFdsList size: " << pollFdsList.size() << std::endl;
	socketFd_to_ServerBlock[newConnection] = socketFd_to_ServerBlock[fd];
	requests[newConnection] = Request();
}

void receiveRequest(int fd)
{
	char buffer[BUFFER_SIZE];
	std::string fullRequest;
	int ret_total = 0;

	ssize_t return_value = recv(fd, buffer, BUFFER_SIZE, 0);
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
	req.setRequestState(PARSING_DONE);
}

static void	resetEvent(int fd)
{
	for (std::vector<struct pollfd>::iterator it = pollFdsList.begin(); it != pollFdsList.end(); it++)
	{
		if (it->fd == fd)
		{
			it->events = POLLIN;
			break;
		}
	}
}

void	sendResponse(int fd, Configuration &config)
{
	// printRequest(requests[fd]);
	static int i = 0;
	std::cout << "Sending response (" << i++ << ")" << std::endl;
	responses[fd] = Response(requests[fd]);
	startTimeForFd[fd] = std::time(0);
	std::string generatedResponse = responses[fd].getResponse(config);
	// std::cout << "fd : " << fd << std::endl;
	int bytes_sent = send(fd, generatedResponse.c_str(), generatedResponse.size(), 0);
	if (bytes_sent == -1)
	{
		std::cout << "ERROR SEND" << std::endl;
		perror("send");
		return;
	}
	resetEvent(fd);
	requests[fd].setRequestState(PROCESSED);
	// requests[fd].clearRequest();
}

int findCount(int fd)
{
	int count = 0;
	for (size_t i = 0; i < listenFds.size(); i++)
	{
		if (listenFds[i] == fd)
			count++;
	}
	return count;
}

static void handleSIGINT(int sig)
{
	(void)sig;
	std::cout << BLUE << "\n  { SIGINT received, stopping server }" << SET << std::endl;
	running = false;
	for(std::map<int, ServerBlock>::iterator it = socketFd_to_ServerBlock.begin(); it != socketFd_to_ServerBlock.end(); it++)
		close(it->first);
}

static void	printRequests(std::map<int, Request> &requests)
{
	std::cout << "Requests:" << std::endl;
	for (std::map<int, Request>::iterator it = requests.begin(); it != requests.end(); it++)
	{
		std::cout << "fd: " << it->first << " state: " << it->second.getRequestState() << std::endl;
	}
}

void checkTimeouts(std::map<int, std::time_t> &startTimeForFd)
{
	std::time_t currentTime = std::time(0);
	std::map<int, std::time_t>::iterator it = startTimeForFd.begin();
	while (it != startTimeForFd.end())
	{
		if (currentTime - it->second > 5)
		{
			rmFromPollWatchlist(it->first);
			socketFd_to_ServerBlock.erase(it->first);
			requests[it->first].clearRequest();
			requests.erase(it->first);
			close(it->first);
			startTimeForFd.erase(it++);
		}
		else
			it++;
	}
}

void runWebServer(Configuration &config)
{
	int	n = 0;
	int	nfds;
	int pollFdsListSize;

	initServer(config);
	signal(SIGINT, handleSIGINT);
	while (running)
	{
		nfds = poll(pollFdsList.data(), pollFdsList.size(), TIMEOUT);
		if (nfds < 0)
			break;
		if (nfds == 0)
			std::cout << GREEN << "Waiting for connection..." << SET << std::endl;

		pollFdsListSize = pollFdsList.size();
		int j = 0;
		for (int i = 0; i < pollFdsListSize && j < nfds; i++)
		{
			struct pollfd	pfd = pollFdsList[i];

			if (pfd.revents == 0 || pfd.fd == -1)
				continue;
			j++;
			if (pfd.revents & POLLIN)
			{
				if (findCount(pfd.fd) > 0)
					acceptConnection(pfd.fd);
				else
					receiveRequest(pfd.fd);
			}
			if (requests[pfd.fd].getParsingState() == PARSING_DONE && pfd.revents & POLLOUT)
				sendResponse(pfd.fd, config);
			// printPollFdsList(pollFdsList);
			if (requests[pfd.fd].getRequestState() == PROCESSED)
			{
				pollFdsList.erase(pollFdsList.begin() + i);
				i--;
				pollFdsListSize--;
			}
			if (pfd.revents & POLLHUP)
			{
				std::cout << "fd " << pfd.fd << " disconnected" << std::endl;
				pollFdsList.erase(pollFdsList.begin() + i);
			}
		}
		checkTimeouts(startTimeForFd);
	}
	std::cout << "Server stopped" << std::endl;
	for(std::map<int, ServerBlock>::iterator it = socketFd_to_ServerBlock.begin(); it != socketFd_to_ServerBlock.end(); it++)
		close(it->first);
}
