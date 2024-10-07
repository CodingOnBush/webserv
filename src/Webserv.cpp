#include "../include/Webserv.hpp"
#include "Webserv.hpp"

bool						g_running = true;
std::map<int, Connection>	g_connections;
std::vector<struct pollfd>	g_pfds;

static void	addNewConnection(int fd, bool isListener)
{
	Connection	conn;

	conn.isListener = isListener;
	conn.startTime = std::time(0);
	conn.req = Request();
	conn.res = Response();
	g_connections.insert(std::pair<int, Connection>(fd, conn));
}

static void	addNewPfd(int fd, int events)
{
	struct pollfd	pfd;

	pfd.fd = fd;
	pfd.events = events;
	pfd.revents = 0;
	g_pfds.push_back(pfd);
}

static void	closeAllFds()
{
	std::vector<struct pollfd>::iterator	it;

	for (std::map<int, Connection>::iterator it = g_connections.begin(); it != g_connections.end(); it++)
	{
		if (it->first > 0)
			close(it->first);
	}
	g_connections.clear();
	g_pfds.clear();
}

static int	createSocket(int port)
{
	struct sockaddr_in	addr;
	int 				opt = 1;
	int 				newSocketFd;

	newSocketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (newSocketFd < 0)
		return (perror("socket"), -1);
	if (setsockopt(newSocketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		return (perror("setsockopt"), close(newSocketFd), -1);
	memset((char*)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	if (bind(newSocketFd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		return (perror("bind"), close(newSocketFd), -1);
	if (listen(newSocketFd, MAX_CLIENTS) < 0)
		return (perror("listen"), close(newSocketFd), -1);
	return (newSocketFd);
}

static void	acceptNewConnection(struct pollfd &pfd)
{
	int	newFd;
	int	opt = 1;
	int	optname = SO_REUSEADDR | SO_REUSEPORT | SO_KEEPALIVE;
	
	newFd = accept(pfd.fd, NULL, NULL);
	if (newFd < 0)
	{
		perror("accept");
		return;
	}
	if (fcntl(newFd, F_SETFL, O_NONBLOCK) < 0)
	{
		close(newFd);
		perror("fcntl");
		return;
	}
	if (setsockopt(newFd, SOL_SOCKET, optname, &opt, sizeof(opt)) < 0)
	{
		close(newFd);
		perror("setsockopt");
		return;
	}
	addNewPfd(newFd, POLLIN);
	addNewConnection(newFd, false);
	std::cout << "New connection on fd " << newFd << " accepted at " << std::time(0) << std::endl;
}

void	receiveRequest(struct pollfd &pfd)
{
	std::stringstream	ss;
	char				buffer[BUFFER_SIZE];
	int					bytesReceived;
	
	bytesReceived = recv(pfd.fd, buffer, BUFFER_SIZE, 0);
	if (bytesReceived < 0)
	{
		perror("recv");
		return;
	}
	pfd.events = POLLOUT | POLLHUP;
	if (bytesReceived == 0)
	{
		g_connections[pfd.fd].req.setRequestState(RECEIVED);
		return ;
	}
	ss.write(buffer, bytesReceived);
	g_connections[pfd.fd].req.parseRequest(ss);
	g_connections[pfd.fd].startTime = std::time(0);
	std::cout << "Received request from fd " << pfd.fd << " at " << std::time(0) << std::endl;
}

static void	sendResponse(struct pollfd &pfd, Configuration &config)
{
	std::string	response;
	int			bytes;

	g_connections[pfd.fd].res = Response(g_connections[pfd.fd].req);
	g_connections[pfd.fd].startTime = std::time(0);
	response = g_connections[pfd.fd].res.getResponse(config);
	bytes = send(pfd.fd, response.c_str(), response.size(), 0);
	if (bytes < 0)
	{
		perror("send");
		return;
	}
	pfd.events = POLLIN;
	g_connections[pfd.fd].res.clearResponse();
	g_connections[pfd.fd].req.clearRequest();
}

static void	handleSIGINT(int sig)
{
	(void)sig;
	std::cout << BLUE << "\n  { SIGINT received, stopping server }" << SET << std::endl;
	g_running = false;
}

static void	checkTimeouts()
{
	std::vector<struct pollfd>	pfdsToRemove;

	for(std::vector<struct pollfd>::iterator it = g_pfds.begin(); it != g_pfds.end(); it++)
	{
		if (g_connections[it->fd].isListener)
			continue;
		if (std::time(0) - g_connections[it->fd].startTime >= 2)
		{
			g_connections.erase(it->fd);
			close(it->fd);
			pfdsToRemove.push_back(*it);
			std::cout << "fd " << it->fd << " timed out" << std::endl;
		}
	}
	int	size = g_pfds.size();
	for (int i = 0; i < size; i++)
	{
		for (std::vector<struct pollfd>::iterator it = pfdsToRemove.begin(); it != pfdsToRemove.end(); it++)
		{
			if (g_pfds[i].fd == it->fd)
			{
				g_pfds.erase(g_pfds.begin() + i);
				size--;
				break;
			}
		}
	}

}

static void	createListeningFds(Configuration &config)
{
	std::set<Hostport>	pairs;
	Servers				servers;
	
	servers = config.getServerBlocks();
	for (Servers::iterator it = servers.begin(); it != servers.end(); it++)
	{
		if (pairs.find(it->hostPort) != pairs.end())
			continue;
		pairs.insert(it->hostPort);
		int	fd = createSocket(it->hostPort.second);
		if (fd < 0)
			continue;
		addNewConnection(fd, true);
		addNewPfd(fd, POLLIN);
	}
}

void runWebServer(Configuration &config)
{
	createListeningFds(config);
	signal(SIGINT, handleSIGINT);
	while(g_running)
	{
		int nfds = poll(g_pfds.data(), g_pfds.size(), TIMEOUT);
		if (nfds < 0)
			break;
		if (nfds == 0)
			std::cout << GREEN << "Waiting for connection..." << SET << std::endl;
		for (std::vector<struct pollfd>::iterator it = g_pfds.begin(); it != g_pfds.end(); it++)
		{
			if (it->revents != 0 && it->fd > 0)
			{
				if (it->revents & POLLIN)
				{
					if (g_connections[it->fd].isListener)
						acceptNewConnection(*it);
					else
						receiveRequest(*it);
				}
				if (it->revents & POLLOUT)
				{
					sendResponse(*it, config);
				}
				if (it->revents & POLLHUP)
				{
					// close(it->fd);
					// g_pfds.erase(it);
					std::cout << "fd " << it->fd << " disconnected" << std::endl;
				}
			}
		}
		checkTimeouts();
	}
	closeAllFds();
	std::cout << "Bye!" << std::endl;
}
