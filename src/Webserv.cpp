#include "../include/Webserv.hpp"
#include "Webserv.hpp"

std::vector<struct pollfd> pollFdsList;

std::map<int, Connection> connections;

std::set<int> listenFds;

bool running = true;

static void	closeFd(int fd)
{
	if (fd == -1)
		return;
	if (fcntl(fd, F_GETFD) != -1 || errno != EBADF)
		close(fd);
}

static void addNewClientAndConnection(int fd, bool isListener, bool isActive)
{
	struct pollfd newPollFd;

	memset(&newPollFd, 0, sizeof(newPollFd));
	newPollFd.fd = fd;
	newPollFd.events = POLLIN | POLLOUT;
	pollFdsList.push_back(newPollFd);

	Connection conn;

	conn.fd = fd;
	conn.isListener = isListener;
	conn.startTime = std::time(0);
	conn.req = Request();
	conn.res = Response();
	conn.isActive = isActive;
	connections.insert(std::pair<int, Connection>(fd, conn));
}

static int createServerSocket(int port)
{
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr;
	int opt = 1;

	if (serverSocket == -1)
		return (perror("socket"), -1);
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		return (perror("setsockopt"), closeFd(serverSocket), -1);
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	if (bind(serverSocket, (struct sockaddr *)&addr, sizeof(addr)) == -1)
		return (perror("bind"), closeFd(serverSocket), -1);
	if (listen(serverSocket, 10) == -1)
		return (perror("listen"), closeFd(serverSocket), -1);
	return (serverSocket);
}

static void initiateWebServer(Configuration &config)
{
	std::vector<ServerBlock> servers = config.getServerBlocks();
	std::set<Hostport> hostPorts;

	listenFds.clear();
	for (std::vector<ServerBlock>::iterator it = servers.begin(); it != servers.end(); it++)
	{
		int serverSocket;

		std::cout << "(" << it->hostPort.first << ":" << it->hostPort.second << ")" << std::endl;
		if (hostPorts.find(it->hostPort) != hostPorts.end())
			continue;
		serverSocket = createServerSocket(it->hostPort.second);
		if (serverSocket == -1)
			continue;
		listenFds.insert(serverSocket);
		hostPorts.insert(it->hostPort);
	}
	for (std::set<int>::iterator it = listenFds.begin(); it != listenFds.end(); it++)
		addNewClientAndConnection(*it, true, false);
}

static void acceptConnection(int fd)
{
	int clientFd = accept(fd, NULL, NULL);
	int opt = 1;

	std::cout << "Accepting connection on fd: " << fd << std::endl;
	if (clientFd < 0)
	{
		perror("accept");
		return;
	}
	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0)
	{
		closeFd(clientFd);
		perror("fcntl");
		return;
	}
	if (setsockopt(clientFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
	{
		closeFd(clientFd);
		perror("setsockopt");
		return;
	}
	addNewClientAndConnection(clientFd, false, true);
	std::cout << "New client connected on fd: " << clientFd << std::endl;
}

static std::vector<struct pollfd>::iterator	closeConnection(std::vector<struct pollfd>::iterator &it)
{
	std::cout << "Closing connection on fd: " << it->fd << std::endl;
	closeFd(it->fd);
	if (it != pollFdsList.end())
		return (pollFdsList.erase(it));
	return (it);
}

static int receiveRequest(int fd)
{
	char buffer[BUFFER_SIZE];
	Request &req = connections[fd].req;
	ssize_t bytes;
	std::stringstream ss;

	memset(buffer, 0, BUFFER_SIZE);
	bytes = recv(fd, buffer, BUFFER_SIZE, 0);
	if (bytes < 0)
	{
		std::cout << "Salut" << std::endl;
		return FAILURE;
	}
	buffer[bytes] = '\0';
	// std::cout << "[" << buffer << "]" << std::endl;
	std::cout << "request received" << std::endl;
	if (bytes == 0)
	{
		std::cout << "Connection closed : " << fd << std::endl;
		// closeConnection(fd);
		return FAILURE;
	}
	ss.write(buffer, bytes);
	req.parseRequest(ss);
	return SUCCESS;
}

static void sendResponse(int fd, Configuration &config)
{
	std::string str;

	connections[fd].res = Response(connections[fd].req);
	// printRequest(connections[fd].req);
	str = connections[fd].res.getResponse(config);
	send(fd, str.c_str(), str.size(), 0);
	std::cout << "Response sent" << std::endl;
	connections[fd].req.clearRequest();
	connections[fd].startTime = std::time(0);
	connections[fd].res.clearResponse();
}

static void handleSIGINT(int sig)
{
	(void)sig;
	std::cout << BLUE << "\nSIGINT received, stopping server" << SET << std::endl;
	running = false;
}

static void printPollFds()
{
	std::cout << "POLLFDS :" << std::endl;
	for (std::vector<struct pollfd>::iterator it = pollFdsList.begin(); it != pollFdsList.end(); it++)
	{
		std::cout
			<< "{fd: " << it->fd << ", events: "
			<< it->events << ", revents: "
			<< ((it->revents & POLLIN) ? "POLLIN | " : "X | ")
			<< ((it->revents & POLLOUT) ? "POLLOUT" : "X")
			<< "}"
			<< std::endl;
	}
	std::cout << std::endl;
}

static void printPfd(std::vector<struct pollfd>::iterator it)
{
	std::cout
		<< "PFD: {fd: " << it->fd << ", events: "
		<< it->events << ", revents: "
		<< ((it->revents & POLLIN) ? "POLLIN | " : "X | ")
		<< ((it->revents & POLLOUT) ? "POLLOUT" : "X")
		<< "}"
		<< std::endl;
}

static void printConnections()
{
	std::cout << "CONNECTIONS :" << std::endl;
	for (std::map<int, Connection>::iterator it = connections.begin(); it != connections.end(); it++)
	{
		std::cout
			<< "{fd: " << it->first << ", isListener: "
			<< it->second.isListener << ", startTime: "
			<< it->second.startTime << "}"
			<< std::endl;
	}
	std::cout << std::endl;
}

void runWebServer(Configuration &config)
{
	std::string wait[] = {"⠋", "⠙", "⠸", "⠴", "⠦", "⠇"};
	int timeout = 1000;
	int n = 0;
	std::vector<int> toClose;

	memset(&listenFds, 0, sizeof(listenFds));
	initiateWebServer(config);
	printPollFds();
	signal(SIGINT, handleSIGINT);
	while (running)
	{
		int nfds = poll(pollFdsList.data(), pollFdsList.size(), timeout);
		if ((nfds < 0 && errno != EINTR) || running == false)
		{
			std::cout << "COUCOUCOU" << std::endl;
			break;
		}
		if (nfds == 0)
		{
			std::cout << GREEN << "Server running " << wait[n++] << SET << "\r" << std::flush;
			if (n == 6)
				n = 0;
			continue;
		}

		int j = 0;
		for (std::vector<struct pollfd>::iterator it = pollFdsList.begin(); it != pollFdsList.end() && j < nfds;)
		{
			if (it->revents == 0)
			{
				++it;
				continue;
			}
			j++;
			if (it->revents & POLLIN)
			{
				if (listenFds.find(it->fd) != listenFds.end())
				{
					acceptConnection(it->fd);
					std::cout << "NEW CONNECTION FD : " << it->fd << std::endl;
				}
				else
				{
					std::cout << "RECEIVING REQUEST ON FD : " << it->fd << std::endl;
					if (receiveRequest(it->fd) == FAILURE)
					{
						// toClose.push_back(it->fd);
						// closeConnection(it);
						closeFd(it->fd);
						it->fd = -1;
						++it;
						continue;
					}
				}
			}
			if (it->revents & POLLOUT)
			{
				if (connections[it->fd].req.getParsingState() == PARSING_DONE)
					sendResponse(it->fd, config);
			}
			++it;
		}
		for(std::vector<struct pollfd>::iterator it = pollFdsList.begin(); it != pollFdsList.end();)
		{
			if (it->fd == -1)
				it = pollFdsList.erase(it);
			else
				++it;
		}
	}
	for (std::vector<struct pollfd>::iterator it = pollFdsList.begin(); it != pollFdsList.end(); it++)
		closeFd(it->fd);
	std::cout << "Server stopped" << std::endl;
}
