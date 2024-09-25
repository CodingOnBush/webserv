#include "Webserv.hpp"

int	main(int ac, char **av)
{
	std::string	fileName = "webserv.conf";

	if (ac == 2)
		fileName = av[1];
	try
	{
		Configuration config(fileName);
		// config.printConfig();
		// runWebserver(config);
		initiateWebServer(config);
		runWebserver(config);
	}
	catch (std::exception &e)
	{
		std::cerr << "Error : " << e.what() << std::endl;
		return (1);
	}
	return (0);
}