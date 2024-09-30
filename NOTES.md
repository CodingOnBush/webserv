
// int	main(int ac, char **av)
// {
// 	const std::string	defaultConfigPath = "./config/webserv.conf";

// 	try
// 	{
// 		Configuration config((ac == 2) ? av[1] : defaultConfigPath);
// 		config.printConfig();
// 		webserv(config);
// 	}
// 	catch (std::exception &e)
// 	{
// 		std::cerr << "Error : " << e.what() << std::endl;
// 		return (1);
// 	}
// 	return (0);
// }