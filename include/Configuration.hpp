#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

/*
# Configuration File’s Structure
nginx consists of modules which are controlled by directives specified
in the configuration file. Directives are divided into simple directives
and block directives. A simple directive consists of the name and
parameters separated by spaces and ends with a semicolon (;).
A block directive has the same structure as a simple directive,
but instead of the semicolon it ends with a set of additional
instructions surrounded by braces ({ and }). If a block directive can
have other directives inside braces, it is called a
context (examples: server or location).

The rest of a line after the # sign is considered a comment.
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cstddef>
#include <sstream>

enum http_method {
	GET,
	POST,
	DELETE
};

struct BodySize {
	std::string	value;
	std::string	unit;
};

struct LocationBlock {
	bool								exactMatch;
	std::string							path;// file or directory
	std::string							root;
	std::string							alias;
	BodySize							clientMaxBodySize;
	bool								autoindex;
	std::vector<std::string>			indexes;
	std::map<std::string, std::string>	redirects;// {code, address}
	bool								pathInfo;
	std::map<std::string, std::string>	cgiParams;// {extension, file}
	std::string							uploadLocation;
	std::vector<http_method>			methods;// GET, POST, DELETE by default
};

struct ServerBlock {
	int									port;// from listen directive
	std::string							host;// from listen directive
	std::vector<std::string>			serverNames;// maybe none or more
	std::string							root;
	std::map<std::string, std::string>	errorPages;// {error code, uri}
	BodySize							clientMaxBodySize;
	std::vector<LocationBlock>			locationBlocks;
};

class Configuration
{
	private:
		std::string					m_configFile;// maybe remove it later
		std::vector<ServerBlock>	m_serverBlocks;
		// std::vector<int>			_ports; // for vic's part
		// and more
	public:
		Configuration(std::string const &t_configFile);
		~Configuration();
		// std::vector<int>			getPorts() const;
};

#endif // CONFIGURATION_HPP
