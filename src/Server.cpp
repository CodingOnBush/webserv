#include "Server.hpp"

Server::Server()
{
}

Server::~Server()
{
}

int Server::createSocket(int socket_fd)
{
	int opt = 1;
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	try
	{
		if (socket_fd < 0)
			throw std::runtime_error("socket() failed");
	}
	catch (std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
	try
	{
		if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
			throw std::runtime_error("setsockopt() failed");;
	}
	catch (std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return socket_fd;
}

void Server::setSocketNonBlocking(int socket_fd)
{
	try
	{
		if (fcntl(socket_fd, F_SETFL, O_NONBLOCK) < 0)
			throw std::runtime_error("fcntl() failed");
	}
	catch (std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
}

void Server::bindAndListen(int socket_fd)
{
	try
	{
		if (bind(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
			throw std::runtime_error("bind() failed");
	}
	catch (std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
	try
	{
		if (listen(socket_fd, MAX_CLIENTS) < 0) // 32 is the maximum size of the queue of pending connections
			throw std::runtime_error("listen() failed");
	}
	catch (std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
}

std::vector<int> Server::setUpSockets(std::vector<int> ports)
{
	std::vector<int> servers_fd;
	int socket_fd;

	for (std::vector<int>::iterator it = ports.begin(); it != ports.end(); it++)
	{
		socket_fd = createSocket(socket_fd);
		setSocketNonBlocking(socket_fd);
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = INADDR_ANY;
		servaddr.sin_port = htons(*it);
		bindAndListen(socket_fd);
		servers_fd.push_back(socket_fd);
	}
	return servers_fd;
}

int Server::createEpoll(std::vector<int> ports)
{
	int epoll_fd = epoll_create(MAX_EVENTS + ports.size());
	try
	{
		if (epoll_fd < 0)
			throw std::runtime_error("epoll_create() failed");
	}
	catch (std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return epoll_fd;
}

void Server::addToInterestList(int epoll_fd, epoll_event ev, std::vector<int>::iterator it)
{
	ev.events = EPOLLIN;
	ev.data.fd = *it;
	try
	{
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, *it, &ev) < 0)
			throw std::runtime_error("epoll_ctl() failed");
	}
	catch (std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
}

int fd_is_server(int fd, std::vector<int> sockets_fd)
{
	for (std::vector<int>::iterator it = sockets_fd.begin(); it != sockets_fd.end(); it++)
	{
		if (fd == *it)
			return (*it);
	}
	return 0;
}

void Server::readingLoop(int epoll_fd, epoll_event ev, epoll_event *events)
{
	int nb_fds = 0;
	int timeout = 1000;
	int j = 0;

	while (1)
	{
		if ((nb_fds = epoll_wait(epoll_fd, events, MAX_EVENTS, timeout)) < 0)
			throw std::runtime_error("epoll_wait() failed");
		else if (nb_fds == 0)
		{
			std::cout << "Waiting for connection" << std::endl;
		}
		for (int i = 0; i < nb_fds; i++)
		{
			int server = 0;
			try
			{
				if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP)
				{
					close(events[i].data.fd);
					throw std::runtime_error("epoll error");
				}
			}
			catch  (std::exception &e)
			{
				std::cerr << "Error: " << e.what() << std::endl;
			}
			if ((server = fd_is_server(events[i].data.fd, this->sockets_fd)) > 0)
			{
				acceptConnection(server, epoll_fd, ev);
			}
			else if (events[i].events & EPOLLIN)
			{
				std::string fullRequest = receiveRequest(events[i].data.fd, ev, epoll_fd);
				Request req(fullRequest);
				break;
			}
			else if (events[j].events & EPOLLOUT)
			{
				sendResponse(events[j].data.fd, ev, epoll_fd, req);
				break;
			}
		}
	}
}

void Server::acceptConnection(int server, int epoll_fd, epoll_event ev)
{
	int new_connection = accept(server, NULL, NULL);
	try 
	{
		if (new_connection < 0)
			throw std::runtime_error("accept() failed");
	}
	catch (std::exception &e) 
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
	std::cout << "New client connection to server accepted" << std::endl;
	setSocketNonBlocking(new_connection);
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = new_connection;
	try
	{
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_connection, &ev) < 0)
		{
			close(new_connection);
			throw std::runtime_error("epoll_ctl() failed");
		}
	}
	catch (std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
}

std::string Server::receiveRequest(int fd, epoll_event ev, int epoll_fd)
{
	int return_value = 0;
	char buffer[BUFFER_SIZE];
	std::string fullRequest;

	memset(buffer, 0, BUFFER_SIZE);
	while ((return_value = recv(fd, buffer, BUFFER_SIZE - 1, 0)) > 0)
	{
		fullRequest.append(buffer, return_value);
	}
	// Request request(fullRequest);
	// request.printRequest(request);
	ev.events = EPOLLOUT;
	ev.data.fd = fd;
	epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
	return (fullRequest);
}

void Server::sendResponse(int fd, epoll_event ev, int epoll_fd, Request req)
{
	(void)req;
	try {
		if (send(fd, response.c_str(), response.size(), 0) < 0)
			throw std::runtime_error("send() failed");
	}
	catch (std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
	std::cout << "Response sent" << response << std::endl;
	ev.events = EPOLLIN;
	ev.data.fd = fd;
	close(fd);
	epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
}

void Server::startServer(std::vector<int> ports)
{
	this->sockets_fd = setUpSockets(ports);
	int epoll_fd = createEpoll(ports);
	epoll_event ev;					// fds to monitor
	epoll_event events[MAX_EVENTS]; // incoming events

	for (std::vector<int>::iterator it = sockets_fd.begin(); it != sockets_fd.end(); it++)
		addToInterestList(epoll_fd, ev, it);

	readingLoop(epoll_fd, ev, events);
}

void Server::setResponse(std::string response)
{
	if (response.empty())
		this->response = response;
	else
		this->response += response;
}
