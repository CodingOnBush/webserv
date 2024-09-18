#include "Webserv.hpp"

int	main(int ac, char **av)
{
	const std::string	defaultPath = "./config/webserv.conf";

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
			// config.printConfig();
			webserv(config);
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
			// config.printConfig();
			webserv(config);
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