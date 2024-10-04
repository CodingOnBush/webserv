#include "../include/Webserv.hpp"
#include "Webserv.hpp"

std::map<int, std::time_t> startTimeForFd;


std::map<std::pair<std::string, int>, int>	socketsToPorts;// unused in this code...
std::map<int, std::vector<ServerBlock> >	serversToFd;
std::map<int, std::time_t>					fdToTimeoutCheck;
std::vector<int>							listenFds;
std::vector<struct pollfd>					pollFdsList;
std::map<int, Request>						requests;
std::map<int, Response> 					responses;
bool										running = true;

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

static void	printSocketsToPorts()
{
	std::cout << "SocketsToPorts (" << socketsToPorts.size() << "):" << std::endl;
	for (std::map<std::pair<std::string, int>, int>::iterator it = socketsToPorts.begin(); it != socketsToPorts.end(); it++)
	{
		std::cout << it->first.first << ":" << it->first.second << "(host:port) -> socket: " << it->second << std::endl;
	}
}

static void	printServersToFd()
{
	std::cout << "ServersToFd (" << serversToFd.size() << "):" << std::endl;
	for (std::map<int, std::vector<ServerBlock> >::iterator it = serversToFd.begin(); it != serversToFd.end(); it++)
	{
		ServerBlock	server = it->second[0];
		std::cout << "fd: " << it->first << " server port: " << server.port << std::endl;
	}
}

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
	int 				serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in	addr;
	int 				opt = 1;

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

void	initiateWebServer(Configuration &config)
{
	std::vector<ServerBlock>	servers = config.getServerBlocks();

	for (std::vector<ServerBlock>::iterator it = servers.begin(); it != servers.end(); it++)
	{
		std::pair<std::string, int>	hostPort = std::make_pair(it->host, it->port);
		int					 		serverSocket;

		// std::cout << "(" << it->host << ":" << it->port << ")"<< std::endl;
		if (socketsToPorts.find(hostPort) != socketsToPorts.end())
			serverSocket = socketsToPorts[hostPort];
		else
		{
			serverSocket = createServerSocket(it->port);
			if (serverSocket == -1)
				continue;
			socketsToPorts[hostPort] = serverSocket;
		}
		serversToFd[serverSocket].push_back(*it);
	}
	// printServersToFd();
	// printSocketsToPorts();
	for (std::map<int, std::vector<ServerBlock> >::iterator it = serversToFd.begin(); it != serversToFd.end(); it++)
		listenFds.push_back(it->first);
	// printListenFds();
	for (std::vector<int>::iterator it = listenFds.begin(); it != listenFds.end(); it++)
	{
		struct pollfd pfd;
		pfd.fd = *it;
		pfd.events = POLLIN | POLLOUT | POLLHUP;
		pfd.revents = 0;
		pollFdsList.push_back(pfd);
	}
	// printPollFdsList(pollFdsList);
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
	std::cout << "PollFdsList size: " << pollFdsList.size() << std::endl;
	serversToFd[newConnection] = serversToFd[fd];
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
}

void sendResponse(int fd, Configuration &config)
{
	// printRequest(requests[fd]);
	static int i = 0;
	// std::cout << "Sending response (" << i++ << ")" << std::endl;
	Response resp(requests[fd]);
	responses[fd] = resp;
	startTimeForFd[fd] = std::time(0);
	std::string generatedResponse = responses[fd].getResponse(config);
	int bytes_sent = send(fd, generatedResponse.c_str(), generatedResponse.size(), 0);
	if (bytes_sent == -1)
	{
		std::cout << "ERROR SEND" << std::endl;
		perror("send");
		return;
	}
	requests[fd].setRequestState(PROCESSED);
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
			serversToFd.erase(it->first);
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
	const std::string	wait[] = {"⠋", "⠙", "⠸", "⠴", "⠦", "⠇"};
	int					n = 0;

	initiateWebServer(config);
	signal(SIGINT, handleSIGINT);
	while (running)
	{
		int nfds = poll(&pollFdsList[0], pollFdsList.size(), TIMEOUT);
		if (nfds < 0)
			break;
		if (nfds == 0)
		{
			// std::cout << GREEN << "  { Waiting for connection " << wait[n++ % 6] << " }" << SET << "\r" << std::flush;
			std::cout << GREEN << "Waiting for connection..." << SET << std::endl;
			// if (n == 6)
			// 	n = 0;
		}
		// else if (nfds > 0)
		// 	std::cout << GREEN << "Connection received!" << SET << std::endl;

		int j = 0;
		int pollFdsListSize = pollFdsList.size();
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
			if (requests[pfd.fd].getRequestState() == PROCESSED)
			{
				requests[pfd.fd].clearRequest();
				requests.erase(pfd.fd);
				rmFromPollWatchlist(pfd.fd);
			}
			if (pfd.revents & POLLHUP)
			{
				rmFromPollWatchlist(pfd.fd);
				serversToFd.erase(pfd.fd);
				requests[pfd.fd].clearRequest();
				requests.erase(pfd.fd);
				close(pfd.fd);
			}
			checkTimeouts(startTimeForFd);
			// printPollFdsList(pollFdsList);
		}
	}
	for(std::map<int, std::vector<ServerBlock> >::iterator it = serversToFd.begin(); it != serversToFd.end(); it++)
		close(it->first);
}
