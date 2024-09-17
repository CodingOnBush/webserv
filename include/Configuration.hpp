#pragma once

// #include <bits/stdc++.h> // for std::stringstream
#include <fstream>
#include <vector>
#include <map>
#include <cstddef>
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib> // for std::atoi
// std::find
#include <algorithm>

#define CRLF "\r\n" // carriage return line feed
#define LF '\n'
#define CR '\r'

enum http_method
{
	GET,
	POST,
	DELETE,
	UNKNOWN
};

struct BodySize
{
	std::string value;
	std::string unit;
};

struct LocationBlock {
	std::string							path;// file or directory
	std::string							alias;
	std::string							root;
	BodySize							clientMaxBodySize;
	int									bodySize;
	bool								autoindex;
	bool								autoindexDone;
	bool								pathInfo;
	std::vector<std::string>			indexes;
	std::map<std::string, std::string>	errorPages;
	std::string							uploadLocation;
	std::map<int, std::string>			redirects;// {code, address}
	bool                               	redirection;
	std::map<std::string, std::string>	cgiParams;// {extension, file}
	std::vector<http_method>			methods;// GET, POST, DELETE by default
};

struct ServerBlock {
	int									port;// from listen directive
	std::string							host;// from listen directive
	std::vector<std::string>			serverNames;// maybe none or more
	std::string							root;
	BodySize							clientMaxBodySize;
	int									bodySize;
	bool								autoindex;
	std::vector<std::string>			indexes;
	std::map<std::string, std::string>	errorPages;// {error code, uri}
	std::map<int, std::string>			redirects;// {code, address}
	bool                               	redirection;
	std::map<std::string, std::string>	cgiParams;// {extension, file}
	std::vector<http_method>			methods;// GET, POST, DELETE by default

	std::vector<LocationBlock>			locationBlocks;

	bool								listenHasBeenSet;
};

class Configuration
{
	private:
		// maybe remove it later
		std::string 				m_configFile;
		// all the content of the config file
		std::stringstream 			m_content;
		// all the server blocks parsed from the config file
		std::vector<ServerBlock>	m_serverBlocks;
		int							curlyBrackets;
		// std::vector<int>			_ports; // for vic's part
		// and more

		void		parseServerBlock(std::stringstream &ss);
		void		parseLocationBlock(ServerBlock &serverBlock, std::string const &locationLine, std::stringstream &ss);
		void		parseServerDirective(std::string const &line, ServerBlock &serverBlock);
		void		parseLocationDirective(std::string &line, LocationBlock &locationBlock);

	public:
		// std::vector<ServerBlock>			m_serverBlocks;
		Configuration();
		Configuration(std::string const &t_configFile);
		~Configuration();
		// std::vector<int>			getPorts() const;

		// getters (We need getter for server_names, ports, etc)
		std::vector<ServerBlock> const &getServerBlocks() const;
		//...
		
		// tools
		const int			getBodySize(BodySize const &bodySize);
		std::vector<int>	getPorts() const;// maybe to remove
		void				printConfig() const;
		//...
};
