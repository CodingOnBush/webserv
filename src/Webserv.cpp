#include "../include/Webserv.hpp"
#include "Webserv.hpp"

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
		int	serverSocket;

		std::cout << "(" << it->hostPort.first << ":" << it->hostPort.second << ")"<< std::endl;
		if (socketsToPorts.find(it->hostPort) != socketsToPorts.end())
			serverSocket = socketsToPorts[it->hostPort];
		else
		{
			serverSocket = createServerSocket(it->hostPort.second);
			if (serverSocket == -1)
				continue;
			socketsToPorts[it->hostPort] = serverSocket;
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

void runWebServer(Configuration &config)
{
	int			timeout = 500;
	std::string	wait[] = {"⠋", "⠙", "⠸", "⠴", "⠦", "⠇"};
	int			n = 0;

	initiateWebServer(config);
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

// bool						g_running = true;
// std::map<int, Connection>	g_connections;
// std::vector<struct pollfd>	g_pfds;

// static int	createListener(int port)
// {
// 	int fd = socket(AF_INET, SOCK_STREAM, 0);
// 	if (fd == -1)
// 	{
// 		std::cerr << "Failed to create socket" << std::endl;
// 		return -1;
// 	}
// 	int opt = 1;
// 	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
// 	{
// 		std::cerr << "Failed to set socket options" << std::endl;
// 		close(fd);
// 		return -1;
// 	}
// 	struct sockaddr_in addr;
// 	addr.sin_family = AF_INET;
// 	addr.sin_port = htons(port);
// 	addr.sin_addr.s_addr = INADDR_ANY;
// 	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
// 	{
// 		std::cerr << "Failed to bind socket" << std::endl;
// 		close(fd);
// 		return -1;
// 	}
// 	if (listen(fd, 10) == -1)
// 	{
// 		std::cerr << "Failed to listen on socket" << std::endl;
// 		close(fd);
// 		return -1;
// 	}
// 	return fd;
// }

// void addConnection(int fd, const Connection &connection, short events)
// {
// 	// Ajouter à la map g_connections
// 	g_connections[fd] = connection;

// 	// Ajouter à la liste g_pfds
// 	struct pollfd pfd;
// 	pfd.fd = fd;
// 	pfd.events = events;
// 	pfd.revents = 0;
// 	g_pfds.push_back(pfd);

// 	std::cout << "Added connection with fd " << fd << " to g_connections and g_pfds." << std::endl;
// }

// void removeConnection(int fd)
// {
// 	// Vérifier si le descripteur existe dans g_connections
// 	if (g_connections.find(fd) != g_connections.end())
// 	{
// 		g_connections.erase(fd); // Supprimer de la map
// 		std::cout << "Removed connection with fd " << fd << " from g_connections." << std::endl;
// 	}

// 	// Supprimer de g_pfds
// 	for (std::vector<struct pollfd>::iterator it = g_pfds.begin(); it != g_pfds.end(); ++it)
// 	{
// 		if (it->fd == fd)
// 		{
// 			g_pfds.erase(it); // Supprimer du vecteur
// 			std::cout << "Removed fd " << fd << " from g_pfds." << std::endl;
// 			break;
// 		}
// 	}

// 	// Fermer le descripteur de fichier
// 	close(fd);
// 	std::cout << "Closed file descriptor " << fd << "." << std::endl;
// }

// static void printConnections()
// {
// 	std::cout << "Connections:" << std::endl;
// 	if (g_connections.empty())
// 	{
// 		std::cout << "No connections" << std::endl;
// 		return;
// 	}
// 	for (std::map<int, Connection>::iterator it = g_connections.begin(); it != g_connections.end(); ++it)
// 	{
// 		std::cout << "fd: " << it->first << ", isListener: " << it->second.isListener << ", startTime: " << it->second.startTime << std::endl;
// 	}
// }

// static void printPfds()
// {
// 	std::cout << "Pfds:" << std::endl;
// 	if (g_pfds.empty())
// 	{
// 		std::cout << "No pfds" << std::endl;
// 		return;
// 	}
// 	for (std::vector<struct pollfd>::iterator it = g_pfds.begin(); it != g_pfds.end(); ++it)
// 	{
// 		std::cout << "fd: " << it->fd << ", events: " << it->events << ", revents: " << it->revents << std::endl;
// 	}
// }

// static void	removeAllConnections()
// {
// 	for (std::map<int, Connection>::iterator it = g_connections.begin(); it != g_connections.end(); ++it)
// 	{
// 		close(it->first);
// 	}
// 	g_connections.clear();
// 	g_pfds.clear();
// }

// static void acceptConnection(int listener)
// {
// 	struct sockaddr_in client_addr;
// 	socklen_t client_len = sizeof(client_addr);
// 	int client_fd = accept(listener, (struct sockaddr *)&client_addr, &client_len);
// 	if (client_fd == -1)
// 	{
// 		std::cerr << "Failed to accept new connection" << std::endl;
// 		return;
// 	}
// 	Connection conn;

// 	conn.isListener = false;
// 	conn.startTime = std::time(0);
// 	addConnection(client_fd, conn, POLLIN);
// 	std::cout << "Accepted new connection on fd " << client_fd << std::endl;
// }

// static void	receiveData(int fd)
// {
// 	char buffer[1024];
// 	ssize_t bytes_read = recv(fd, buffer, sizeof(buffer) - 1, 0);
// 	if (bytes_read <= 0)
// 	{
// 		if (bytes_read == 0)
// 			std::cout << "Connection closed by peer on fd " << fd << std::endl;
// 		else
// 			std::cerr << "Error reading from fd " << fd << std::endl;
// 		removeConnection(fd);
// 	}
// 	else
// 	{
// 		buffer[bytes_read] = '\0';
// 		std::cout << "Received data on fd " << fd << ": " << buffer << std::endl;
// 		for (std::vector<struct pollfd>::iterator it = g_pfds.begin(); it != g_pfds.end(); ++it)
// 		{
// 			if (it->fd == fd)
// 			{
// 				it->events |= POLLOUT;
// 				break;
// 			}
// 		}
// 	}
// }

// static void handlePollout(int fd)
// {
// 	std::cout << "Handling POLLOUT on fd " << fd << std::endl;

// 	// Example response message
// 	std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, world!";
	
// 	ssize_t bytes_sent = send(fd, response.c_str(), response.size(), 0);
// 	if (bytes_sent == -1)
// 	{
// 		std::cerr << "Error sending data to fd " << fd << std::endl;
// 		removeConnection(fd);
// 	}
// 	else
// 	{
// 		std::cout << "Sent data to fd " << fd << std::endl;

// 		// After sending the response, we might no longer need to keep checking for POLLOUT, so we remove it
// 		for (std::vector<struct pollfd>::iterator it = g_pfds.begin(); it != g_pfds.end(); ++it)
// 		{
// 			if (it->fd == fd)
// 			{
// 				it->events &= ~POLLOUT; // Disable POLLOUT flag
// 				break;
// 			}
// 		}
// 	}
// }

// void runWebServer(Configuration &config)
// {
// 	Servers servers = config.getServerBlocks();

// 	for (Servers::iterator it = servers.begin(); it != servers.end(); ++it)
// 	{
// 		ServerBlock &server = *it;
// 		int port = server.hostPort.second;
// 		int fd = createListener(port);
// 		if (fd == -1)
// 			continue;
// 		Connection conn;

// 		conn.isListener = true;
// 		conn.startTime = std::time(0);
// 		addConnection(fd, conn, POLLIN);
// 		std::cout << "Listening on port " << port << std::endl;
// 	}

// 	printConnections();
// 	printPfds();

// 	while (g_running)
// 	{
// 		int nfds = poll(&g_pfds[0], g_pfds.size(), TIMEOUT);
// 		if (nfds == -1)
// 		{
// 			perror("poll");
// 			break;
// 		}
// 		if (nfds == 0)
// 		{
// 			std::cout << "Waiting for connections..." << std::endl;
// 			continue;
// 		}

// 		for (std::vector<struct pollfd>::iterator it = g_pfds.begin(); it != g_pfds.end(); ++it)
// 		{
// 			if (it->revents == 0)
// 				continue;

// 			if (it->revents & POLLIN)
// 			{
// 				if (g_connections[it->fd].isListener)
// 					acceptConnection(it->fd);
// 				else
// 					receiveData(it->fd);
// 			}
// 			if (it->revents & POLLOUT)
// 				handlePollout(it->fd);
// 			if (it->revents & POLLHUP)
// 				removeConnection(it->fd);
// 		}
// 	}

// 	// Cleanup before exit
// 	removeAllConnections();
// 	printConnections();
// 	printPfds();
// }

