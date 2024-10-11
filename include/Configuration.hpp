#pragma once

#include <fstream>
#include <vector>
#include <map>
#include <cstddef>
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <algorithm>

#define CRLF "\r\n"
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
	std::string							path;
	std::string							alias;
	std::string							root;
	BodySize							clientMaxBodySize;
	int									bodySize;
	bool								autoindex;
	bool								autoindexDone;
	bool								pathInfo;
	std::vector<std::string>			indexes;
	std::vector<std::string>			cgiExtensions;
	std::map<std::string, std::string>	errorPages;
	std::string							uploadLocation;
	std::map<int, std::string>			redirects;
	bool                               	redirection;
	std::map<std::string, std::string>	cgiParams;
	std::vector<http_method>			methods;
};

typedef std::pair<std::string, int> Hostport;

struct ServerBlock {

	Hostport							hostPort;
	std::vector<std::string>			serverNames;
	std::string							root;
	BodySize							clientMaxBodySize;
	int									bodySize;
	bool								autoindex;
	std::vector<std::string>			indexes;
	std::vector<std::string>			cgiExtensions;
	std::map<std::string, std::string>	errorPages;
	std::map<int, std::string>			redirects;
	bool                               	redirection;
	std::map<std::string, std::string>	cgiParams;
	std::vector<http_method>			methods;

	std::vector<LocationBlock>			locationBlocks;

	bool								listenHasBeenSet;
};

class Configuration
{
	private:
		std::string 				m_configFile;
		std::vector<ServerBlock>	m_serverBlocks;
		int							curlyBrackets;

		void		parseServerBlock(std::stringstream &ss);
		void		parseLocationBlock(ServerBlock &serverBlock, std::string const &locationLine, std::stringstream &ss);
		void		parseServerDirective(std::string const &line, ServerBlock &serverBlock);
		void		parseLocationDirective(std::string &line, LocationBlock &locationBlock);

	public:
		Configuration();
		Configuration(std::string const &t_configFile);
		~Configuration();

		std::vector<ServerBlock> const	&getServerBlocks() const;
		int								getBodySize(BodySize const &bodySize);
		void							printConfig() const;
};

void	initLocationBlock(LocationBlock &locationBlock);