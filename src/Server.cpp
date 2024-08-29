#include "../include/Server.hpp"

// Server::Server()
// {
// }

// Server::~Server()
// {
// }

// int Server::createSocket(int socket_fd)
// {
// 	int opt = 1;
// 	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
// 	if (socket_fd < 0)
// 		throw std::runtime_error("socket() failed");
// 	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
// 		throw std::runtime_error("setsockopt() failed");;
// 	return socket_fd;
// }

// void Server::setSocketNonBlocking(int socket_fd)
// {
// 	if (fcntl(socket_fd, F_SETFL, O_NONBLOCK) < 0)
// 		throw std::runtime_error("fcntl() failed");
// }

// void Server::bindAndListen(int socket_fd)
// {
// 	if (bind(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
// 			throw std::runtime_error("bind() failed");
// 	if (listen(socket_fd, MAX_CLIENTS) < 0) // 32 is the maximum size of the queue of pending connections
// 			throw std::runtime_error("listen() failed");
// }

// std::vector<int> Server::setUpSockets(std::vector<int> ports)
// {
// 	std::vector<int> servers_fd;
// 	int socket_fd;

// 	for (std::vector<int>::iterator it = ports.begin(); it != ports.end(); it++)
// 	{
// 		socket_fd = createSocket(socket_fd);
// 		setSocketNonBlocking(socket_fd);
// 		servaddr.sin_family = AF_INET;
// 		servaddr.sin_addr.s_addr = INADDR_ANY;
// 		servaddr.sin_port = htons(*it);
// 		bindAndListen(socket_fd);
// 		servers_fd.push_back(socket_fd);
// 	}
// 	return servers_fd;
// }

// int Server::createEpoll(std::vector<int> ports)
// {
// 	int epoll_fd = epoll_create(MAX_EVENTS + ports.size());
// 	if (epoll_fd < 0)
// 			throw std::runtime_error("epoll_create() failed");
// 	return epoll_fd;
// }

// void Server::addToInterestList(int epoll_fd, epoll_event ev, std::vector<int>::iterator it)
// {
// 	ev.events = EPOLLIN;
// 	ev.data.fd = *it;
// 	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, *it, &ev) < 0)
// 			throw std::runtime_error("epoll_ctl() failed");
// }

// int fd_is_server(int fd, std::vector<int> sockets_fd)
// {
// 	for (std::vector<int>::iterator it = sockets_fd.begin(); it != sockets_fd.end(); it++)
// 	{
// 		if (fd == *it)
// 			return (*it);
// 	}
// 	return 0;
// }

// void Server::readingLoop(int epoll_fd, epoll_event ev, epoll_event *events)
// {
// 	int nb_fds = 0;
// 	int timeout = 1000;
// 	int j = 0;

// 	while (1)
// 	{
// 		if ((nb_fds = epoll_wait(epoll_fd, events, MAX_EVENTS, timeout)) < 0)
// 			throw std::runtime_error("epoll_wait() failed");
// 		else if (nb_fds == 0)
// 		{
// 			std::cout << "Waiting for connection" << std::endl;
// 		}
// 		for (int i = 0; i < nb_fds; i++)
// 		{
// 			int server = 0;
// 			if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP)
// 				{
// 					close(events[i].data.fd);
// 					throw std::runtime_error("epoll error");
// 				}
// 			if ((server = fd_is_server(events[i].data.fd, this->sockets_fd)) > 0)
// 			{
// 				acceptConnection(server, epoll_fd, ev);
// 			}
// 			else if (events[i].events & EPOLLIN)
// 			{
// 				std::string fullRequest = receiveRequest(events[i].data.fd, ev, epoll_fd);
// 				Request req(fullRequest);
// 				break;
// 			}
// 			else if (events[j].events & EPOLLOUT)
// 			{
// 				sendResponse(events[j].data.fd, ev, epoll_fd, req);
// 				break;
// 			}
// 		}
// 	}
// }

// void Server::acceptConnection(int server, int epoll_fd, epoll_event ev)
// {
// 	int new_connection = accept(server, NULL, NULL);
// 	if (new_connection < 0)
// 			throw std::runtime_error("accept() failed");
// 	std::cout << "New client connection to server accepted" << std::endl;
// 	setSocketNonBlocking(new_connection);
// 	ev.events = EPOLLIN | EPOLLET;
// 	ev.data.fd = new_connection;
// 	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_connection, &ev) < 0)
// 		{
// 			close(new_connection);
// 			throw std::runtime_error("epoll_ctl() failed");
// 		}
// }

// std::string Server::receiveRequest(int fd, epoll_event ev, int epoll_fd)
// {
// 	int return_value = 0;
// 	char buffer[BUFFER_SIZE];
// 	std::string fullRequest;

// 	memset(buffer, 0, BUFFER_SIZE);
// 	while ((return_value = recv(fd, buffer, BUFFER_SIZE - 1, 0)) > 0)
// 	{
// 		fullRequest.append(buffer, return_value);
// 	}
// 	// Request request(fullRequest);
// 	// request.printRequest(request);
// 	ev.events = EPOLLOUT;
// 	ev.data.fd = fd;
// 	epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
// 	return (fullRequest);
// }

// void Server::sendResponse(int fd, epoll_event ev, int epoll_fd, Request req)
// {
// 	(void)req;
// 	if (send(fd, response.c_str(), response.size(), 0) < 0)
// 			throw std::runtime_error("send() failed");
// 	std::cout << "Response sent" << response << std::endl;
// 	ev.events = EPOLLIN;
// 	ev.data.fd = fd;
// 	close(fd);
// 	epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
// }

// void Server::startServer(std::vector<int> ports)
// {
// 	try
// 	{
// 		this->sockets_fd = setUpSockets(ports);
// 		int epoll_fd = createEpoll(ports);
// 		epoll_event ev;					// fds to monitor
// 		epoll_event events[MAX_EVENTS]; // incoming events

// 		for (std::vector<int>::iterator it = sockets_fd.begin(); it != sockets_fd.end(); it++)
// 			addToInterestList(epoll_fd, ev, it);
		
// 		readingLoop(epoll_fd, ev, events);
// 	}
// 	catch (std::exception &e)
// 	{
// 		std::cerr << "Error: " << e.what() << std::endl;
// 	}
// }

// void Server::setResponse(std::string response)
// {
// 	if (response.empty())
// 		this->response = response;
// 	else
// 		this->response += response;
// }

void Server::setServerSockets(Configuration &config)
{
	for (std::vector<ServerBlock>::iterator it = config.m_serverBlocks.begin(); it != config.m_serverBlocks.end(); it++)
	{
		int socket_fd;
		int opt = 1;
		socket_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (socket_fd < 0)
			throw std::runtime_error("socket() failed");
		if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
			throw std::runtime_error("setsockopt() failed");
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = INADDR_ANY;
		servaddr.sin_port = htons(it->port);
		if (bind(socket_fd, &servaddr, sizeof(servaddr)) < 0)
			throw std::runtime_error("bind() failed");
		if (listen(socket_fd, MAX_CLIENTS) < 0)
			throw std::runtime_error("listen() failed");
		this->ServersToPortSockets.insert(std::pair<ServerBlock, int>(*it, socket_fd));
	}
}

void Server::addToPoll(int fd)
{
	struct pollfd pfd = (struct pollfd){fd, POLLIN | POLLOUT , 0};
	this->pollFds.push_back(pfd);
}

void Server::monitorFds()
{
	for (std::map<ServerBlock, int>::iterator it = this->ServersToPortSockets.begin(); it != this->ServersToPortSockets.end(); it++)
	{
		this->listenFds.insert(it->second);
	}
	for (std::set<int>::iterator it = this->listenFds.begin(); it != this->listenFds.end(); it++)
	{
		addToPoll(*it);
	}
}

Server::Server(const Configuration &config)
{
	try
	{
		setServerSockets(config);
		monitorFds();
		bindAndListen();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	
}

Server::~Server()
{
}

void Server::setSocketNonBlocking(int socket_fd)
{
	if (fcntl(socket_fd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("fcntl() failed");
}

void Server::acceptNewConnection(int fd)
{
	int opt = 1;
	int new_connection = accept(fd, NULL, NULL);
	if (new_connection < 0)
		throw std::runtime_error("accept() failed");
	std::cout << "New client connection to server accepted" << std::endl;
	setSocketNonBlocking(new_connection);
	setsockopt(new_connection, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
	addToPoll(new_connection);
}

void Server::startServer()
{
	int nfds = 0;
	int timeout = 1000;
	int j = 0;

	while (1)
	{
		nfds = poll(&this->pollFds[0], this->pollFds.size(), timeout);
		if (nfds < 0)
			throw std::runtime_error("poll() failed");
		if (nfds == 0)
		{
			std::cout << "Waiting for connection" << std::endl;
		}
		for (int i = 0; i < pollfds.size(); i++)
		{
			int fd = pollfds[i].fd;

			if (pollfds[i].revents == 0)
				continue;
			if (pollfds[i].revents & POLLIN)
			{
				if ( listenFds.count(fd) == 1 )
				{
					acceptNewConnection(currentFd);
				}
				else
				{
					readRequest(currentFd);
				}
			}
		}
	}
}