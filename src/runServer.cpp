// #include "../include/runServer.hpp"

// std::map<std::pair<std::string, int>, int>	socketsToPorts;
// std::map<int, std::vector<ServerBlock> >	serversToFd;
// std::vector<int>							listenFds;
// std::vector<struct pollfd>					pollFdsList;
// bool										running = true;

// std::map<int, Request>						requests;
// std::map<int, Response>						responses;

// // handleSignals
// void	handleSIGINT(int signum)
// {
// 	running = false;
// }

// static int	createServerSocket(int port)
// {
// 	int 				serverSocket = socket(AF_INET, SOCK_STREAM, 0);
// 	int 				opt = 1;
// 	struct sockaddr_in	serverAddr;

// 	if (serverSocket == -1)
// 		return (perror("socket"), -1);
// 	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
// 		return (perror("setsockopt"), close(serverSocket), -1);
// 	memset(&serverAddr, 0, sizeof(serverAddr));
// 	serverAddr.sin_family = AF_INET;
// 	serverAddr.sin_addr.s_addr = INADDR_ANY;
// 	serverAddr.sin_port = htons(port);
// 	if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
// 		return (perror("bind"), close(serverSocket), -1);
// 	if (listen(serverSocket, 10) == -1)
// 		return (perror("listen"), close(serverSocket), -1);
// 	return (serverSocket);
// }

// void	acceptConnection(int fd)
// {
// 	// std::cout << "acceptConnection" << std::endl;
// 	struct sockaddr_in cli;
// 	socklen_t len = sizeof(cli);
// 	int new_connection = accept(fd, (struct sockaddr *)&cli, &len);
// 	if (new_connection < 0)
// 		throw std::runtime_error("accept() failed");
// 	std::cout << "Connection accepted : " << new_connection << std::endl;
// 	if (fcntl(new_connection, F_SETFL, O_NONBLOCK) == -1)
// 		throw std::runtime_error("fcntl() failed");
// 	int opt = 1;
// 	if (setsockopt(new_connection, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
// 		throw std::runtime_error("setsockopt() failed");
// 	struct pollfd pfd = (struct pollfd){new_connection, POLLIN | POLLOUT | POLLHUP, 0};
// 	pollFdsList.push_back(pfd);
// 	serversToFd[new_connection] = serversToFd[fd];
// 	requests[new_connection] = Request();
// }

// void	receiveRequest(int fd)
// {
// 	// std::cout << "receiveRequest" << std::endl;
// 	char buffer[BUFFER_SIZE];
// 	std::string fullRequest;
// 	int ret_total = 0;

// 	ssize_t return_value = recv(fd, buffer, BUFFER_SIZE, 0);
// 	if (return_value == -1)
// 		return;
	
// 	Request &req = requests[fd];
// 	if (return_value == 0)
// 	{
// 		req.setRequestState(RECEIVED);
// 		return;
// 	}
// 	std::stringstream ss;
// 	ss.write(buffer, return_value);
// 	req.parseRequest(ss);
// }

// void	sendResponse(int fd, Configuration &config)
// {
// 	std::cout << "sendResponse" << std::endl;
// 	printRequest(requests[fd]);
// 	Response resp(requests[fd]);
// 	responses[fd] = resp;
// 	std::string generatedResponse = responses[fd].getResponse(config);
// 	int bytes_sent = send(fd, generatedResponse.c_str(), generatedResponse.size(), 0);
// 	requests[fd].setRequestState(PROCESSED);
// }

// void	initWebserv(Configuration &config)
// {
// 	std::vector<ServerBlock>	servers = config.getServerBlocks();

// 	for (std::vector<ServerBlock>::iterator it = servers.begin(); it != servers.end(); it++)
// 	{
// 		std::pair<std::string, int>	hostPort = std::make_pair(it->host, it->port);
// 		int					 		serverSocket;

// 		std::cout << "(" << it->host << ":" << it->port << ")"<< std::endl;
		
// 		if (socketsToPorts.find(hostPort) != socketsToPorts.end())
// 			serverSocket = socketsToPorts[hostPort];
// 		else
// 		{
// 			serverSocket = createServerSocket(it->port);
// 			if (serverSocket == -1)
// 				continue;
// 			socketsToPorts[hostPort] = serverSocket;
// 		}
// 		serversToFd[serverSocket].push_back(*it);
// 	}
// 	for (std::map<int, std::vector<ServerBlock> >::iterator it = serversToFd.begin(); it != serversToFd.end(); it++)
// 		listenFds.push_back(it->first);
// 	for (std::vector<int>::iterator it = listenFds.begin(); it != listenFds.end(); it++)
// 	{
// 		struct pollfd pfd = (struct pollfd){*it, POLLIN | POLLOUT | POLLHUP, 0};
// 		pollFdsList.push_back(pfd);
// 	}
// }

// void	printPollFd(struct pollfd &pfd)
// {
// 	std::cout << "fd = " << pfd.fd << std::endl;
// 	std::cout << "events = " << pfd.events << std::endl;
// 	std::cout << "revents = " << pfd.revents << std::endl;
// }

// int findCount(int fd)
// {
// 	int count = 0;
// 	for (size_t i = 0; i < listenFds.size(); i++)
// 	{
// 		if (listenFds[i] == fd)
// 			count++;
// 	}
// 	return count;
// }

// void	runServer(Configuration &config)
// {
// 	int	timeout = 1000;

// 	initWebserv(config);
// 	signal(SIGINT, handleSIGINT);
// 	while(running)
// 	{
// 		// std::cout << "loop" << std::endl;
// 		int ret = poll(&pollFdsList[0], pollFdsList.size(), timeout);
// 		if (ret == -1)
// 		{
// 			perror("poll");
// 			break;
// 		}
// 		// std::cout << "ret = " << ret << std::endl;
// 		if (ret == 0)
// 			std::cout << "Waiting for requests or connections..." << std::endl;
// 		for (std::vector<struct pollfd>::iterator it = pollFdsList.begin(); it != pollFdsList.end(); it++)
// 		{
// 			if (it->revents == 0)
// 				continue;
// 			if ((it->revents & POLLIN) == POLLIN)
// 			{
// 				std::cout << "POLLIN" << std::endl;
// 				if (findCount(it->fd) == 1)
// 					acceptConnection(it->fd);
// 				else
// 					receiveRequest(it->fd);
// 			}
// 			if ((it->revents & POLLOUT) == POLLOUT)
// 			{
// 				std::cout << "POLLOUT" << std::endl;
// 				// if (requests[it->fd].getParsingState() == PARSING_DONE)
// 				// 	sendResponse(it->fd, config);
// 			}
// 			if ((it->revents & POLLHUP) == POLLHUP || requests[it->fd].getParsingState() == PROCESSED)
// 			{
// 				std::cout << "POLLHUP" << std::endl;
// 				// pollFdsList.erase(it);
// 				// serversToFd[it->fd].clear();
// 				// requests.erase(it->fd);
// 				// close(it->fd);
// 			}
// 		}
// 	}
// 	for (std::map<std::pair<std::string, int>, int>::iterator it = socketsToPorts.begin(); it != socketsToPorts.end(); it++)
// 		close(it->second);
// }
