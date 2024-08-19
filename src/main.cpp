#include <iostream>

int main(int argc, char **argv)
{
	(void)argv;
	if (argc == 2)
	{
		std::cout << "Staring webserv!" << std::endl;
		return (0);
	}
	std::cout << "Invalid input! Usage: ./webserv [configuration file]" << std::endl;
	return 0;
}