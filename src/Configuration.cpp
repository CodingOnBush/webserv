#include "../include/Configuration.hpp"
#include <sstream>

static bool	isDirective(std::string const &word)
{
	return word == "listen" || word == "server_name" || word == "root"
		|| word == "error_page" || word == "client_max_body_size";
}

static void	parseConfigFile(std::vector<std::string> words)
{
	int	i = 0;
	if (words.size() == 0)
		throw std::runtime_error("Empty configuration file");
	if (words[0] != "server")
		throw std::runtime_error("Configuration file must start with 'server' directive");
	if (words[1] != "{")
		throw std::runtime_error("Missing '{' after 'server' directive");
	i = 2;
	while (i < words.size())
	{
		if (isDirective(words[i]))
			i += parseDirective(words, i);
		else if (words[i] == "location")
			i += parseLocation(words, i);
		else if (words[i] == "}")
			break;
		else
			throw std::runtime_error("Unknown directive '" + words[i] + "'");
	}
}

Configuration::Configuration(std::string const &filename) : configFile(filename)
{
	std::stringstream			buffer;
	std::ifstream				file(configFile);
	std::vector<std::string>	words;

	if (!file)
		throw std::runtime_error("Cannot open file " + configFile);
	buffer << file.rdbuf();
	file.close();
	std::string	fileContent = buffer.str();
	std::istringstream	iss(fileContent);
	std::string	word;
	while (iss >> word) {
		words.push_back(word);
    }
	parseConfigFile(words);
	// createPortList();
}

Configuration::~Configuration()
{
}
