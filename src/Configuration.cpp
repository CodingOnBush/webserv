#include "../include/Configuration.hpp"

Configuration::Configuration(std::string const &filename)
{
	std::ifstream	file;

	file.open(filename.c_str());
	if (!file.is_open())
		throw std::runtime_error("Cannot open file " + filename);
	std::string line;
	while (std::getline(file, line))
	{
		std::cout << line << std::endl;
	}
	file.close();
}

Configuration::~Configuration()
{
}