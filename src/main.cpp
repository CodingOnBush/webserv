#include "Configuration.hpp"
#include "Webserv.hpp"

#define MAX_EV 4096


// int main(int ac, char **av)
// {
// 	// signal(SIGINT, SIG_DFL);
// 	// parse the config file and set the server ports through the vector
	
// 	// if (ac != 2)
// 	// {
// 	// 	std::cerr << "Usage: ./webserv <config_file>" << std::endl;
// 	// 	return (1);
// 	// }
// 	// try {
// 	// 	/*
// 	// 	vic need this :
// 	// 	std::vector<int> ports;
// 	// 	*/
// 	// 	Configuration	config(av[1]);
// 	// 	config.printConfig();
// 	// }
// 	// catch (std::exception &e) {
// 	// 	std::cerr << "Error: " << e.what() << std::endl;
// 	// 	return (1);
// 	// }
// 	(void)ac;
// 	(void)av;
// 	Server server;
// 	std::vector<int> ports;
// 	ports.push_back(8080);
// 	ports.push_back(8081);

// 	server.setResponse("HTTP/1.1 200 OK\n");
// 	server.setResponse("Content-Type: text/html\r\n");
// 	server.setResponse("Content-Length: 13\n\n");
// 	server.setResponse("Hello World !\r\n\r\n");
// 	// server.startServer(ports);
	
// 	// server.CloseServer();
// 	return (0);
// }

int main(int ac, char **av)
{
	(void)ac;
	(void)av;
	try
	{
		Configuration config("config.txt");
		initiateWebServer(config);
		runWebserver();
	}
	catch (std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return (1);
	}
	return (0);
}
