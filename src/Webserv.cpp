#include "../include/Webserv.hpp"
#include "Webserv.hpp"

std::map<int, Connection> connections;

std::set<int> listenFds;

bool running = true;

static void closeFd(int fd)
{
	if (fd == -1)
		return;
	if (fcntl(fd, F_GETFD) != -1 || errno != EBADF)
		close(fd);
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

static void initiateWebServer(std::vector<pollfd> &pfds, Configuration &config)
{
	std::vector<ServerBlock> servers = config.getServerBlocks();
	std::set<Hostport> hostPorts;

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
	{
		pollfd pfd;

		pfd.fd = *it;
		pfd.events = POLLIN;
		pfd.revents = 0;
		pfds.push_back(pfd);
	}
}

static void acceptConnection(std::vector<pollfd> &pfds, int fd)
{
	int clientFd = accept(fd, NULL, NULL);
	int opt = 1;

	std::cout << GREEN << "Accepting connection on fd: " << fd << SET << std::endl;
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
	if (setsockopt(clientFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		closeFd(clientFd);
		perror("setsockopt SO_REUSEADDR");
		return;
	}

	if (setsockopt(clientFd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
	{
		closeFd(clientFd);
		perror("setsockopt SO_REUSEPORT");
		return;
	}

	if (pfds.capacity() == pfds.size())
		pfds.reserve(pfds.size() + 10);

	pollfd newPfd;

	newPfd.fd = clientFd;
	newPfd.events = POLLIN | POLLOUT;
	newPfd.revents = 0;
	pfds.push_back(newPfd);

	Connection conn;

	conn.fd = clientFd;
	conn.isListener = false;
	conn.startTime = std::time(0);
	conn.req = Request();
	conn.res = Response();
	conn.isActive = true;
	connections.insert(std::pair<int, Connection>(clientFd, conn));
	std::cout << GREEN << "New client connected on fd: " << clientFd << SET << std::endl;
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
		return FAILURE;
	}
	buffer[bytes] = '\0';
	if (bytes == 0)
	{
		std::cout << YELLOW << "Connection closed : " << fd << SET << std::endl;
		return FAILURE;
	}
	ss.write(buffer, bytes);
	req.parseRequest(ss);
	return SUCCESS;
}

static int	sendResponse(int fd, Configuration &config)
{
	std::string str;
	ssize_t bytes;

	connections[fd].res = Response(connections[fd].req);
	str = connections[fd].res.getResponse(config);
	bytes = send(fd, str.c_str(), str.size(), 0); 
	if (bytes <= 0)
	{
		closeFd(fd);
		return FAILURE;
	}
	connections[fd].req.clearRequest();
	connections[fd].startTime = std::time(0);
	connections[fd].res.clearResponse();
	return SUCCESS;
}

static void handleSIGINT(int sig)
{
	(void)sig;
	std::cout << BLUE << "\nSIGINT received, stopping server" << SET << std::endl;
	running = false;
}

void runWebServer(Configuration &config)
{
	std::string wait[] = {"⠋", "⠙", "⠸", "⠴", "⠦", "⠇"};
	int timeout = 500;
	int n = 0;
	std::vector<pollfd> pollFdsList;
	pollFdsList.reserve(300);
	initiateWebServer(pollFdsList, config);
	signal(SIGINT, handleSIGINT);
	while (running)
	{
		int nfds = poll(pollFdsList.data(), pollFdsList.size(), timeout);
		if ((nfds < 0 && errno != EINTR) || running == false)
			break;
		if (nfds == 0)
		{
			std::cout << GREEN << "Server running " << wait[n++] << SET << CR << std::flush;
			if (n == 6)
				n = 0;
			continue;
		}

		int j = 0;
		for (std::vector<pollfd>::iterator it = pollFdsList.begin(); it != pollFdsList.end();)
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
					acceptConnection(pollFdsList, it->fd);
				}
				else
				{
					if (receiveRequest(it->fd) == FAILURE)
					{
						closeFd(it->fd);
						it = pollFdsList.erase(it);
						continue;
					}
				}
			}
			if (it->revents & POLLOUT)
			{
				if (connections[it->fd].req.getParsingState() == PARSING_DONE)
					if (sendResponse(it->fd, config) == FAILURE)
					{
						closeFd(it->fd);
						it = pollFdsList.erase(it);
						continue;
					}
			}
			if (listenFds.find(it->fd) == listenFds.end())
			{
				if (std::time(0) - connections[it->fd].startTime >= TIMEOUT)
				{
					closeFd(it->fd);
					it = pollFdsList.erase(it);
					continue;
				}
			}
			++it;
		}
	}
	for (std::vector<pollfd>::iterator it = pollFdsList.begin(); it != pollFdsList.end(); it++)
		closeFd(it->fd);
	std::cout << YELLOW << "Server stopped" << SET << std::endl;
}
