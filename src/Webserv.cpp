#include "../include/Webserv.hpp"
#include "Webserv.hpp"

std::vector<struct pollfd>					pollFdsList;

std::map<int, Connection>					connections;

std::map<int, std::vector<ServerBlock> >	serversToFd;
std::map<int, Request>						requests;
std::map<int, Response> 					responses;
bool										running = true;

static void rmFromPollWatchlist(int fd)
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
	if (listen(serverSocket, 10) == -1)
		return (perror("listen"), close(serverSocket), -1);
	return (serverSocket);
}

static void	initiateWebServer(Configuration &config)
{
	std::vector<ServerBlock>	servers = config.getServerBlocks();
	std::set<Hostport>			hostPorts;
	std::set<int>				listenFds;

	for (std::vector<ServerBlock>::iterator it = servers.begin(); it != servers.end(); it++)
	{
		int	serverSocket;

		std::cout << "(" << it->hostPort.first << ":" << it->hostPort.second << ")"<< std::endl;
		if (hostPorts.find(it->hostPort) != hostPorts.end())
			continue;
		serverSocket = createServerSocket(it->hostPort.second);
		if (serverSocket == -1)
			continue;
		serversToFd[serverSocket].push_back(*it);
		hostPorts.insert(it->hostPort);
	}
	for (std::map<int, std::vector<ServerBlock> >::iterator it = serversToFd.begin(); it != serversToFd.end(); it++)
		listenFds.insert(it->first);
	for (std::set<int>::iterator it = listenFds.begin(); it != listenFds.end(); it++)
	{
		struct pollfd pfd;

		memset(&pfd, 0, sizeof(pfd));
		pfd.fd = *it;
		pfd.events = POLLIN;
		pfd.revents = 0;
		pollFdsList.push_back(pfd);
		connections[pfd.fd] = (Connection){true, std::time(0), Request(), Response()};
	}
}

static void	addNewClient(int fd)
{
	struct pollfd newPollFd;

	memset(&newPollFd, 0, sizeof(newPollFd));
	newPollFd.fd = fd;
	newPollFd.events = POLLIN | POLLOUT;
	newPollFd.revents = 0;
	pollFdsList.push_back(newPollFd);
}

static void	acceptConnection(int fd)
{
	int	clientFd = accept(fd, NULL, NULL);
	int	opt = 1;

	if (clientFd < 0)
	{
		perror("accept");
		return;
	}
	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0)
	{
		close(clientFd);
		perror("fcntl");
		return;
	}
	if (setsockopt(clientFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
	{
		close(clientFd);
		perror("setsockopt");
		return;
	}
	addNewClient(clientFd);
	std::cout << "New client connected on fd: " << clientFd << std::endl;
	serversToFd[clientFd] = serversToFd[fd];
	requests[clientFd] = Request();
}

static void	closeConnection(int fd)
{
	std::cout << "Closing connection on fd: " << fd << std::endl;
	close(fd);
	rmFromPollWatchlist(fd);
	serversToFd.erase(fd);
	requests[fd].clearRequest();
	// connections.erase(fd);
}

static void receiveRequest(int fd)
{
	std::string			fullRequest;
	char 				buffer[BUFFER_SIZE];
	Request 			&req = requests[fd];
	ssize_t 			bytes;
	std::stringstream	ss;

	memset(buffer, 0, BUFFER_SIZE);
	bytes = recv(fd, buffer, BUFFER_SIZE, 0);
	if (bytes < 0)
		return;
	buffer[bytes] = '\0';
	// std::cout << "[" << buffer << "]" << std::endl;
	std::cout << "request received" << std::endl;
	if (bytes == 0)
	{
		std::cout << "Connection closed" << std::endl;
		closeConnection(fd);
		return;
	}
	ss.write(buffer, bytes);
	req.parseRequest(ss);
}

static void	sendResponse(int fd, Configuration &config)
{
	Response	resp(requests[fd]);
	std::string	response;

	printRequest(requests[fd]);
	responses[fd] = resp;
	response = responses[fd].getResponse(config);
	send(fd, response.c_str(), response.size(), 0);
	std::cout << "Response sent" << std::endl;
	requests[fd].clearRequest();
	connections[fd].startTime = std::time(0);
	connections[fd].req.clearRequest();
	connections[fd].res.clearResponse();
}

static void handleSIGINT(int sig)
{
	(void)sig;
	std::cout << BLUE << "\nSIGINT received, stopping server" << SET << std::endl;
	running = false;
}

static void	printPollFds()
{
	std::cout << "POLLFDS :" << std::endl;
	for (std::vector<struct pollfd>::iterator it = pollFdsList.begin(); it != pollFdsList.end(); it++)
	{
		std::cout
		<< "{fd: " << it->fd << ", events: " 
		<< it->events << ", revents: " 
		<< ((it->revents & POLLIN) ? "POLLIN | " : "X | ")
		<< ((it->revents & POLLOUT) ? "POLLOUT | " : "X | ")
		<< "}"
		<< std::endl;
	}
	std::cout << std::endl;
}

static void	printPfd(std::vector<struct pollfd>::iterator it)
{
	std::cout 
		<< "PFD: {fd: " << it->fd << ", events: " 
		<< it->events << ", revents: " 
		<< ((it->revents & POLLIN) ? "POLLIN | " : "X | ")
		<< ((it->revents & POLLOUT) ? "POLLOUT | " : "X | ")
		<< "}"
	<< std::endl;
}

static void	checkTimeouts()
{
	std::time_t	now = std::time(0);

	for (std::map<int, Connection>::iterator it = connections.begin(); it != connections.end(); it++)
	{
		if (it->second.isListener)
			continue;
		if (now - it->second.startTime > 5)
		{
			std::cout << "Connection timeout on fd: " << it->first << std::endl;
			closeConnection(it->first);
		}
	}
}

void runWebServer(Configuration &config)
{
	std::string	wait[] = {"⠋", "⠙", "⠸", "⠴", "⠦", "⠇"};
	int			timeout = 1000;
	int			n = 0;

	initiateWebServer(config);
	signal(SIGINT, handleSIGINT);
	while (running)
	{
		int nfds = poll(pollFdsList.data(), pollFdsList.size(), timeout);
		if (nfds < 0 && errno != EINTR)
			break;
		if (nfds == 0)
		{
			std::cout << GREEN << "Waiting for connection " << wait[n++ % 6] << SET << "\r" << std::flush;
			// std::cout << GREEN << "Waiting for connection..." << SET << std::endl;
			if (n == 6)
				n = 0;
		}
		
		int j = 0;
		std::cout << "------------START--------------" << std::endl;
		printPollFds();
		for (std::vector<struct pollfd>::iterator it = pollFdsList.begin(); it != pollFdsList.end() && j < nfds; it++)
		{
			// printPfd(it);
			if (it->revents == 0)
			{
				// std::cout << "No events" << std::endl;
				continue;
			}
			j++;
			if (it->revents & POLLIN)
			{
				if (connections[it->fd].isListener)
					acceptConnection(it->fd);
				else
					receiveRequest(it->fd);
			}
			if (it->revents & POLLOUT)
			{
				if (requests[it->fd].getParsingState() == PARSING_DONE)
					sendResponse(it->fd, config);
			}
		}
		checkTimeouts();
		std::cout << "--------------END----------------" << std::endl;
	}
	for(std::map<int, std::vector<ServerBlock> >::iterator it = serversToFd.begin(); it != serversToFd.end(); it++)
		close(it->first);
	std::cout << "Server stopped" << std::endl;
}
