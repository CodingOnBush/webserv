#include "../include/Webserv.hpp"
#include "Webserv.hpp"

bool			g_running = true;
Connection		g_connections[MAX_EVENTS];
struct pollfd	g_pfds[MAX_EVENTS];// list of all file descriptors to poll

static void	closeAllFds()
{
	for (nfds_t i = 0; i < MAX_EVENTS; i++)
	{
		if (g_pfds[i].fd > 0)
			close(g_pfds[i].fd);
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
	int	newConnection;
	int	opt = 1;
	
	newConnection = accept(pfd.fd, NULL, NULL);
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
	if (setsockopt(newConnection, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		close(newConnection);
		perror("setsockopt");
		return;
	}
	if (setsockopt(newConnection, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
	{
		close(newConnection);
		perror("setsockopt");
		return;
	}
	if (setsockopt(newConnection, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)) < 0)
	{
		close(newConnection);
		perror("setsockopt");
		return;
	}
	g_pfds[newConnection].fd = newConnection;
	g_pfds[newConnection].events = POLLIN | POLLHUP;
	g_connections[newConnection].req = Request();
	g_connections[newConnection].startTime = std::time(0);
	std::cout << "New connection on fd " << newConnection << " accepted at " << std::time(0) << std::endl;
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
	g_pfds[pfd.fd].events = POLLOUT;
	if (bytesReceived == 0)
	{
		g_connections[pfd.fd].req.setRequestState(RECEIVED);
		return ;
	}
	ss.write(buffer, bytesReceived);
	g_connections[pfd.fd].req.parseRequest(ss);
	g_connections[pfd.fd].req.setRequestState(PARSING_DONE);
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
	g_connections[pfd.fd].req.setRequestState(PROCESSED);
	g_connections[pfd.fd].res = Response();
	g_connections[pfd.fd].req = Request();
}

static void	handleSIGINT(int sig)
{
	(void)sig;
	std::cout << BLUE << "\n  { SIGINT received, stopping server }" << SET << std::endl;
	g_running = false;
	closeAllFds();
}

static void	initPfds(struct pollfd *pfds, std::set<int> &listeningFds)
{
	for(nfds_t i = 0; i < MAX_EVENTS; i++)
	{
		if (listeningFds.find(i) != listeningFds.end())
		{
			pfds[i].fd = i;
			pfds[i].events = POLLIN | POLLHUP;
			pfds[i].revents = 0;
		}
		else
			pfds[i].fd = -1;
	}
}

static void	checkTimeouts(nfds_t i, std::set<int> &listeningFds)
{
	if (listeningFds.find(g_pfds[i].fd) == listeningFds.end() && g_pfds[i].fd > 0)
	{
		if (std::time(0) - g_connections[g_pfds[i].fd].startTime >= 5)
		{
			std::cout << "fd " << g_pfds[i].fd << " timed out" << std::endl;
			close(g_pfds[i].fd);
			g_pfds[i].fd = -1;
		}
	}
}

void runWebServer(Configuration &config)
{
	
	std::set<int>	listeningFds;// used only to check if a fd is a server socket

	listeningFds = createListeningFds(config);
	memset(g_pfds, 0, sizeof(g_pfds));
	initPfds(g_pfds, listeningFds);
	signal(SIGINT, handleSIGINT);
	while(g_running)
	{
		int nfds = poll(g_pfds, MAX_EVENTS, TIMEOUT);
		if (nfds < 0)
			break;
		if (nfds == 0)
			std::cout << GREEN << "Waiting for connection..." << SET << std::endl;
		for (nfds_t i = 0; i < MAX_EVENTS; i++)
		{
			// if (g_pfds[i].fd > 0)
			// 	std::cout << "loop through g_pfds[" << i << "].fd = " << g_pfds[i].fd << std::endl;
			if (g_pfds[i].revents != 0 && g_pfds[i].fd > 0)
			{
				if (g_pfds[i].revents & POLLIN)
				{
					if (listeningFds.find(g_pfds[i].fd) != listeningFds.end())
						acceptNewConnection(g_pfds[i]);
					else
						receiveRequest(g_pfds[i]);
				}
				if (g_pfds[i].revents & POLLOUT)
				{
					sendResponse(g_pfds[i], config);
				}
				if (g_pfds[i].revents & POLLHUP)
				{
					std::cout << "fd " << g_pfds[i].fd << " disconnected" << std::endl;
					close(g_pfds[i].fd);
					g_pfds[i].fd = -1;
				}
			}
			checkTimeouts(i, listeningFds);
		}
	}
	closeAllFds();
}
