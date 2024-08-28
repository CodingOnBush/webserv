
#include "Server.hpp"
#include "Configuration.hpp"

#define MAX_EV 4096


int main(int ac, char **av)
{
	// signal(SIGINT, SIG_DFL);
	// parse the config file and set the server ports through the vector
	
	if (ac != 2)
	{
		std::cerr << "Usage: ./webserv <config_file>" << std::endl;
		return (1);
	}
	try {
		/*
		vic need this :
		std::vector<int> ports;
		*/
		Configuration	config(av[1]);
		config.printConfig();
	}
	catch (std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return (1);
	}
	Server server;
	std::vector<int> ports;
	ports.push_back(8080);
	ports.push_back(8081);

	server.SetResponse("HTTP/1.1 200 OK\n");
	server.SetResponse("Content-Type: text/html\r\n");
	server.SetResponse("Content-Length: 13\n\n");
	server.SetResponse("Hello World !\r\n\r\n");
	
	server.startServer(ports);
	// server.CloseServer();
	return (0);
}