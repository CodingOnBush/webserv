#include "../include/Webserv.hpp"
#include "Webserv.hpp"

bool						g_running = true;
std::map<int, Connection>	g_connections;
std::vector<struct pollfd>	g_pfds;

static void	closeAllFds()
{
	for(std::vector<struct pollfd>::iterator it = g_pfds.begin(); it != g_pfds.end(); it++)
	{
		g_connections.erase(it->fd);
		if (it->fd > 0)
			close(it->fd);
		g_pfds.erase(it);
	}
}

static int	createServerSocket(int port)
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

static std::set<int>	createListeningFds(Configuration &config)
{
	std::set<Hostport>	hostPorts;
	std::set<int>		listeningFds;
	Servers				servers;
	
	servers = config.getServerBlocks();
	for (Servers::iterator it = servers.begin(); it != servers.end(); it++)
	{
		ServerBlock	s = *it;
		int			fd;

		if (hostPorts.find(s.hostPort) != hostPorts.end())
			continue;
		fd = createServerSocket(s.hostPort.second);
		if (fd < 0)
			continue;
		hostPorts.insert(s.hostPort);
		listeningFds.insert(fd);
	}
	return (listeningFds);
}

static void	acceptNewConnection(struct pollfd &pfd)
{
	int	newFd;
	int	opt = 1;
	
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
	if (setsockopt(newFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT | SO_KEEPALIVE, &opt, sizeof(opt)) < 0)
	{
		close(newFd);
		perror("setsockopt");
		return;
	}
	struct pollfd	newPfd;

	newPfd.fd = newFd;
	newPfd.events = POLLIN;
	g_pfds.push_back(newPfd);
	g_connections[newFd].req = Request();
	g_connections[newFd].startTime = std::time(0);
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
	int			bytesSent;

	g_connections[pfd.fd].res = Response(g_connections[pfd.fd].req);
	g_connections[pfd.fd].startTime = std::time(0);
	response = g_connections[pfd.fd].res.getResponse(config);
	bytesSent = send(pfd.fd, response.c_str(), response.size(), 0);
	if (bytesSent < 0)
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
	closeAllFds();
}

static void	initPfds(std::vector<struct pollfd> &pfds, std::set<int> &listeningFds)
{
	for (std::set<int>::iterator it = listeningFds.begin(); it != listeningFds.end(); it++)
	{
		struct pollfd	pfd;

		pfd.fd = *it;
		pfd.events = POLLIN;
		pfds.push_back(pfd);
	}
}

static void	checkTimeouts(std::set<int> &listeningFds)
{
	std::vector<struct pollfd>	pfdsToRemove;

	for(std::vector<struct pollfd>::iterator it = g_pfds.begin(); it != g_pfds.end(); it++)
	{
		if (listeningFds.find(it->fd) != listeningFds.end())
			continue;
		if (it->fd == 0 || it->fd == 1 || it->fd == 2)
			continue;
		if (std::time(0) - g_connections[it->fd].startTime > 3)
		{
			std::cout << "fd " << it->fd << " timed out" << std::endl;
			g_connections.erase(it->fd);
			close(it->fd);
			pfdsToRemove.push_back(*it);
		}
	}
}

void runWebServer(Configuration &config)
{
	
	std::set<int>	listeningFds;// used only to check if a fd is a server socket

	listeningFds = createListeningFds(config);
	initPfds(g_pfds, listeningFds);
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
			// if (it->fd > 0)
			// 	std::cout << "loop through g_pfds[" << it - g_pfds.begin() << "].fd = " << it->fd << std::endl;
			if (it->revents != 0 && it->fd > 0)
			{
				if (it->revents & POLLIN)
				{
					if (listeningFds.find(it->fd) != listeningFds.end())
						acceptNewConnection(*it);
					else
						receiveRequest(*it);
				}
				if (it->revents & POLLOUT)
				{
					// if (g_connections[it->fd].req.getRequestState() == PARSING_DONE)
					sendResponse(*it, config);
				}
				if (it->revents & POLLHUP)
				{
					std::cout << "fd " << it->fd << " disconnected" << std::endl;
					close(it->fd);
					g_pfds.erase(it);
				}
			}
		}
		// checkTimeouts(listeningFds);
	}
	closeAllFds();
	std::cout << "Bye!" << std::endl;
}
