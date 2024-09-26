#include "../include/Webserv.hpp"

std::map<std::pair<std::string, int>, int>	socketsToPorts;
std::map<int, std::vector<ServerBlock> >	serversToFd;
std::vector<int>							listenFds;
std::vector<struct pollfd>					pollFdsList;
std::map<int, Request>						requests;
std::map<int, Response> 					responses;
bool										running = true;

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

static void	initSockaddr(struct sockaddr_in &addr, int port)
{
	memset((char*)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
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
	initSockaddr(addr, port);
	if (bind(serverSocket, (struct sockaddr *)&addr, sizeof(addr)) == -1)
		return (perror("bind"), close(serverSocket), -1);
	if (listen(serverSocket, 10) == -1)
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

		std::cout << "(" << it->host << ":" << it->port << ")"<< std::endl;
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
	for (std::map<int, std::vector<ServerBlock> >::iterator it = serversToFd.begin(); it != serversToFd.end(); it++)
		listenFds.push_back(it->first);
	for (std::vector<int>::iterator it = listenFds.begin(); it != listenFds.end(); it++)
	{
		struct pollfd pfd = (struct pollfd){*it, POLLIN | POLLOUT, 0};
		pollFdsList.push_back(pfd);
	}
	requests.clear();
	responses.clear();
}

void	acceptConnection(int fd)
{
	int				newConnection = accept(fd, NULL, NULL);
	int				opt = 1;
	struct pollfd	pfd;
	
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
	pfd.fd = newConnection;
	pfd.events = POLLIN | POLLOUT;
	pfd.revents = 0;
	pollFdsList.push_back(pfd);
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
	std::cout << BLUE << "\nSIGINT received, stopping server" << SET << std::endl;
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

void runWebserver(Configuration &config)
{
	int			timeout = 500;
	std::string	wait[] = {"⠋", "⠙", "⠸", "⠴", "⠦", "⠇"};
	int			n = 0;

	signal(SIGINT, handleSIGINT);
	while (running)
	{
		int nfds = poll(&pollFdsList[0], pollFdsList.size(), timeout);
		if (nfds < 0 && errno != EINTR)
			break;
		if (nfds == 0)
		{
			std::cout << GREEN << "Waiting for connection " << wait[n++ % 6] << SET << "\r" << std::flush;
			n++;
		}
		
		int j = 0;
		for (std::vector<struct pollfd>::iterator it = pollFdsList.begin(); it != pollFdsList.end() && j < nfds; it++)
		{
			if (it->revents == 0)
				continue;
			j++;
			if (it->revents & POLLIN)
			{
				if (findCount(it->fd) == 1)
					acceptConnection(it->fd);
				else
					receiveRequest(it->fd);
			}
			if (it->revents & POLLOUT)
			{
				if (requests[it->fd].getParsingState() == PARSING_DONE)
					sendResponse(it->fd, config);
			}
			if (requests[it->fd].getRequestState() == PROCESSED)
			{
				rmFromPollWatchlist(it->fd);
				serversToFd.erase(it->fd);
				requests[it->fd].clearRequest();
				requests.erase(it->fd);
				close(it->fd);
			}
		}
	}
	for(std::map<int, std::vector<ServerBlock> >::iterator it = serversToFd.begin(); it != serversToFd.end(); it++)
		close(it->first);
	// std::cout << "Server stopped" << std::endl;
}
