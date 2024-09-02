#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cstddef>
#include <sstream>
#include <cstdlib>// for std::atoi

enum http_method {
	GET,
	POST,
	DELETE
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
	RETURN,
	PATH_INFO,
	CGI,
	UPLOAD_LOCATION,
	SET_METHOD,
	ALIAS
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
		// maybe remove it later
		std::string							m_configFile;

		// all the directives easly accessible
		std::map<std::string, directives>	m_directives;

		// all the content of the config file
		std::stringstream					m_content;

		// all the server blocks parsed from the config file
		std::vector<ServerBlock>			m_serverBlocks;

		// std::vector<int>			_ports; // for vic's part
		// and more

		void		initDirectiveMap();
		
		std::string	extractDirective(std::string const &line);
		std::string	extractValue(std::string const &line);
		void		parseServerBlock(std::stringstream &content);
		void		parseLocationBlock(std::stringstream &content, ServerBlock &serverBlock, std::string const &line);
		
		void		parseServerDirective(std::string const &line, ServerBlock &serverBlock);

		void		setLocationValues(std::string const &expression, std::string const &value, LocationBlock &locationBlock);
		void		setServerValues(std::string const &expression, std::string const &value, ServerBlock &serverBlock);

		void		setListen(std::string const &value, ServerBlock &serverBlock);
		void		setName(std::string const &value, ServerBlock &serverBlock);
		void		setErrorPage(std::string const &value, ServerBlock &serverBlock);
		void		setAlias(std::string const &value, LocationBlock &locationBlock);
		void		setAutoindex(std::string const &value, LocationBlock &locationBlock);
		void		setIndex(std::string const &value, LocationBlock &locationBlock);
		void		setRedirect(std::string const &value, LocationBlock &locationBlock);
		void		setPathInfo(std::string const &value, LocationBlock &locationBlock);
		void		setCgi(std::string const &value, LocationBlock &locationBlock);
		void		setUploadLocation(std::string const &value, LocationBlock &locationBlock);
		void		setMethod(std::string const &value, LocationBlock &locationBlock);
		
		void		setServerRoot(std::string const &value, ServerBlock &serverBlock);
		void		setServerClientMaxBodySize(std::string const &value, ServerBlock &serverBlock);

		void		setLocationRoot(std::string const &value, LocationBlock &locationBlock);
		void		setLocationClientMaxBodySize(std::string const &value, LocationBlock &locationBlock);

	public:
		Configuration();
		Configuration(std::string const &t_configFile);
		~Configuration();
		// std::vector<int>			getPorts() const;

		void	printConfig() const;

		// getters
		/*
			We need getter for server_names, ports, etc
		*/
};

#endif // CONFIGURATION_HPP
