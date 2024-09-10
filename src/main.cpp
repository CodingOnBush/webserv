
// #include "Server.hpp"
#include "Configuration.hpp"

#define MAX_EV 4096

int	main(int ac, char **av)
{
	// signal(SIGINT, SIG_DFL);
	// parse the config file and set the server ports through the vector
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
		}
		catch (std::exception &e)
		{
			std::cerr << "Error: " << e.what() << std::endl;
			return (1);
		}
	}
	else
	{
		try
		{
			Configuration config;
			config.printConfig();
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

/*
TODO :
./webserv config.txt
	celui ou on a un servername qui fini pas par un ;
body size only in bytes - changer pour juste avoir un int et pas une struct
servername directive alone - si toute seule comme ca : "server_name" sans le ;
test getLocationPath method

*/