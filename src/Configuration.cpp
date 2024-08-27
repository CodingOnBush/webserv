#include "../include/Configuration.hpp"

// static int	parseLocationBlock(std::vector<std::string> lines, int i)
// {
// 	for(i++; i < (int)lines.size(); i++)
// 	{
// 		// std::cout << "line: " << lines[i] << std::endl;
// 		if (lines[i].rfind("\t\t", 0) == 0)
// 		{
// 			std::cout << "directive : " << lines[i] << std::endl;
// 		}
// 		if (lines[i] == "\t}")
// 			return i + 1;
// 	}
// 	return i;
// }

static int	parseServerBlock(std::vector<std::string> lines, int i)
{
	std::cout << "server block" << std::endl;
	std::cout << "line: [" << lines[i] << "]" << std::endl;
	while(i < (int)lines.size())
	{
		if (lines[i].empty())
		{
			i++;
			continue;
		}
		if (lines[i] == "}")
		{
			std::cout << "end of server block : [" << lines[i] << "]" << std::endl;
			return i;
		}
		if (lines[i].rfind("\tlocation", 0) == 0)
		{
			i += parseServerBlock(lines, i + 1);
		}
		
		// std::cout << "line: " << lines[i] << std::endl;
		// if (lines[i].rfind("\tlocation", 0) == 0)
		// {
		// 	// std::cout << "location block" << std::endl;
		// 	i += parseLocationBlock(lines, i);
		// 	// std::cout << "out of the location block : " << lines[i] << std::endl;
		// }
		// else if (lines[i].rfind("\t", 0) == 0)// create isDirectiveLine
		// {
		// 	std::cout << "directive : " << lines[i] << std::endl;
		// }
		// if (lines[i] == "}")
		// {
		// 	// std::cout << "end of server block : [" << lines[i] << "]" << std::endl;
		// 	return i;
		// }
	}
	return i;
}

static void	parseConfigFile(std::vector<std::string> lines)
{
	if (lines.size() == 0)
		throw std::runtime_error("Empty configuration file");
	for (int i = 0; i < (int)lines.size(); i++)
	{
		if (lines[i] == "server {")
			i += parseServerBlock(lines, i + 1);
		else if (lines[i].empty())
			i++;
		else
			throw std::runtime_error("Unknown directive '" + lines[i] + "'");
		if (lines[i] != "}")
			throw std::runtime_error("Missing '}'");
	}
}

Configuration::Configuration(std::string const &t_configFile) : m_configFile(t_configFile)
{
	std::vector<std::string>	lines;
	std::stringstream			buffer;
	std::ifstream				file(m_configFile.c_str());

	if (!file)
		throw std::runtime_error("Cannot open file " + m_configFile);
	buffer << file.rdbuf();
	file.close();
	std::string line;
	while (std::getline(buffer, line))
	{
		std::stringstream lineStream(line);
		// std::cout << "line: " << line << std::endl;
		lines.push_back(line);
	}
	parseConfigFile(lines);
	/*
	I will remove parseConfigFile next time to improve code
	*/
	// createPortList();
}

Configuration::~Configuration()
{
}
