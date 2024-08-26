#include "../include/Configuration.hpp"

static void	parseConfigFile(std::string const &filename)
{
	std::ifstream	file;

	file.open(filename.c_str());
	if (!file.is_open())
		throw std::runtime_error("Cannot open file " + filename);
	std::string line;
	while (std::getline(file, line))
	{
		std::cout << line << std::endl;
		//
	}
	file.close();
}

Configuration::Configuration(std::string const &filename) : configFile(filename)
{
	parseConfigFile(configFile);
	// createPortList();
}

Configuration::~Configuration()
{
}
