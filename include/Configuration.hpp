#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

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
	DELETE,
	UNKNOWN
};

enum directives {
	LISTEN,
	SERVER_NAME,
	ROOT,
	ERROR_PAGE,
	CLIENT_MAX_BODY_SIZE,
	LOCATION,
	AUTOINDEX,
	INDEX,
	REDIRECT,
	PATH_INFO,
	CGI_PARAM,
	UPLOAD_LOCATION,
	METHOD
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
		std::string							m_configFile;// maybe remove it later
		std::vector<ServerBlock>			m_serverBlocks;

		std::stringstream					m_content;
		std::map<std::string, directives>	m_directives;
		// std::vector<int>			_ports; // for vic's part
		// and more

		void	parseConfigFile();
		void	parseServerBlock(std::stringstream &content);
		void	parseLocationBlock(std::stringstream &content, ServerBlock &serverBlock);
		void	parseLocationDirective(std::string const &line, LocationBlock &locationBlock);
		void	parseServerDirective(std::string const &line, ServerBlock &serverBlock);
		void	setLocationValues(std::string const &expression, std::string const &value, LocationBlock &locationBlock);
		void	setServerValues(std::string const &expression, std::string const &value, ServerBlock &serverBlock);
		void	initDirectiveMap();

	public:
		Configuration(std::string const &t_configFile);
		~Configuration();
		// std::vector<int>			getPorts() const;
		
		void	printConfig() const;

		// getters
};

#endif // CONFIGURATION_HPP
