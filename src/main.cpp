#include "Webserv.hpp"

extern bool run;

static void	signalHandler(int signum)
{
	(void)signum;
	std::cout << std::endl << "SIGINT received, let's shut down the server" << std::endl;
	run = false;
}

int	main(int ac, char **av)
{
	signal(SIGINT, signalHandler);
	if (ac > 2)
	{
		std::cerr << "Usage: ./webserv <config_file>" << std::endl;
		return (1);
	}
	if (ac == 2)
	{
		try
		{
			Configuration config(av[1]);
			config.printConfig();
			initiateWebServer(config);
			runWebserver(config);
			closeSockets();
		}
		catch (std::exception &e)
		{
			std::cerr << "Error : " << e.what() << std::endl;
			return (1);
		}
	}
	else
	{
		try
		{
			Configuration config;
			config.printConfig();
			initiateWebServer(config);
			runWebserver(config);
			closeSockets();
		}
		catch (std::exception &e)
		{
			std::cerr << "Error: " << e.what() << std::endl;
			return (1);
		}
		return (0);
	}
	return (0);
}
