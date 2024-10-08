#include "../include/Webserv.hpp"
#include "Webserv.hpp"

std::vector<struct pollfd>					pollFdsList;

std::map<int, Connection>					connections;

std::map<int, std::vector<ServerBlock> >	serversToFd;
std::map<int, Request>						requests;
std::map<int, Response> 					responses;
bool										running = true;

static void	resetPfd(struct pollfd &pfd)
{
	pfd.events = POLLIN | POLLOUT;
	pfd.revents = 0;
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
	if (listen(serverSocket, 10) == -1)
		return (perror("listen"), close(serverSocket), -1);
	return (serverSocket);
}

void	initiateWebServer(Configuration &config)
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

	// print poll fds :
	for (std::vector<struct pollfd>::iterator it = pollFdsList.begin(); it != pollFdsList.end(); it++)
		std::cout << "pollfd: " << it->fd << std::endl;

	// print listen fds :
	for (std::set<int>::iterator it = listenFds.begin(); it != listenFds.end(); it++)
		std::cout << "listenfd: " << *it << std::endl;

	// // print connections :
	for (std::map<int, Connection>::iterator it = connections.begin(); it != connections.end(); it++)
	{
		std::cout 
			<< "{pfd.fd: " << it->first << "} -> {isListener: " << it->second.isListener << "}" << std::endl;
	}
}

void	acceptConnection(int fd)
{
	int				newClientFd;
	int				opt = 1;
	struct pollfd	pfd;
	
	memset(&pfd, 0, sizeof(pfd));
	newClientFd = accept(fd, NULL, NULL);
	if (newClientFd < 0)
	{
		perror("accept");
		return;
	}
	if (fcntl(newClientFd, F_SETFL, O_NONBLOCK) < 0)
	{
		close(newClientFd);
		perror("fcntl");
		return;
	}
	if (setsockopt(newClientFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
	{
		close(newClientFd);
		perror("setsockopt");
		return;
	}
	pfd.fd = newClientFd;
	pfd.events = POLLIN | POLLOUT;
	pfd.revents = 0;
	pollFdsList.push_back(pfd);
	std::cout << "New client connected on fd: " << newClientFd << std::endl;
	serversToFd[newClientFd] = serversToFd[fd];
	requests[newClientFd] = Request();
	// connections[newClientFd]
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

void receiveRequest(int fd, struct pollfd &pfd)
{
	(void)pfd;
	// static int	i = 0;
	char buffer[BUFFER_SIZE];
	std::string fullRequest;
	int ret_total = 0;

	// std::cout << "i: " << i++ << std::endl;
	memset(buffer, 0, BUFFER_SIZE);
	Request &req = requests[fd];
	ssize_t return_value = recv(fd, buffer, BUFFER_SIZE, 0);
	if (return_value < 0)
	{
		// std::cout << "Error while receiving request" << std::endl;
		// req.setRequestState(RECEIVED);
		return;
	}
	if (return_value == 0)
	{
		std::cout << "Connection closed" << std::endl;
		closeConnection(fd);
		// req.setRequestState(RECEIVED);
		return;
	}
	buffer[return_value] = '\0';
	// std::cout << "[" << buffer << "]" << std::endl;
	// if (return_value == 0)
	// {
	// 	req.setRequestState(RECEIVED);
	// 	return;
	// }
	std::stringstream ss;
	ss.write(buffer, return_value);
	req.parseRequest(ss);

	std::cout << "request received" << std::endl;
}

void sendResponse(int fd, Configuration &config, struct pollfd &pfd)
{
	printRequest(requests[fd]);
	(void)pfd;
	// if (requests[fd].getRequestState() == PROCESSED)
	// {
	// 	// requests[fd].clearRequest();
	// 	return;
	// }
	// printRequest(requests[fd]);
	Response resp(requests[fd]);
	responses[fd] = resp;
	std::string generatedResponse = responses[fd].getResponse(config);
	std::cout << "FD : " << fd << std::endl;
	send(fd, generatedResponse.c_str(), generatedResponse.size(), 0);
	std::cout << "Response sent" << std::endl;
	// requests[fd].setRequestState(PROCESSED);
	requests[fd].clearRequest();
	// pfd.events = POLLIN | PO
	// pfd.revents	= 0;
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
		// std::cout << "polldfds size: " << pollFdsList.size() << std::endl;
		// std::cout << "nfds size: " << nfds << std::endl;
		// printPollFds();
		if (nfds < 0 && errno != EINTR)
			break;
		if (nfds == 0)
		{
			// std::cout << GREEN << "Waiting for connection " << wait[n++ % 6] << SET << "\r" << std::flush;
			std::cout << GREEN << "Waiting for connection..." << SET << std::endl;
			if (n == 6)
				n = 0;
		}
		
		int j = 0;
		// std::cout << "------------START--------------" << std::endl;
		// printPollFds();
		for (std::vector<struct pollfd>::iterator it = pollFdsList.begin(); it != pollFdsList.end() && j < nfds; it++)
		{
			if (it->revents == 0)
			{
				// std::cout << "No events" << std::endl;
				continue;
			}
			// printPfd(it);
			j++;
			if (it->revents & POLLIN)
			{
				if (connections[it->fd].isListener)
					acceptConnection(it->fd);
				else
				{
					// std::cout << "request received : " << requests[it->fd].getRequestState() << std::endl;
					receiveRequest(it->fd, *it);
				}
			}
			if (it->revents & POLLOUT)
			{
				// std::cout << "POLLOUT ici" << std::endl;
				if (requests[it->fd].getParsingState() == PARSING_DONE)
				{
					// std::cout << "parsing done !!!" << std::endl;
					sendResponse(it->fd, config, *it);
				}
			}
		}
		// std::cout << "--------------END----------------" << std::endl;
	}
	for(std::map<int, std::vector<ServerBlock> >::iterator it = serversToFd.begin(); it != serversToFd.end(); it++)
		close(it->first);
	std::cout << "Server stopped" << std::endl;
}
