#include "../include/Configuration.hpp"
#include "Configuration.hpp"

static BodySize	createBodySize(std::string const &value)
{
	BodySize	bodySize;
	std::string	unit;
	std::string	number;

	if (value.empty() || value.at(0) == '\0' || value == "0")
	{
		bodySize.value = "0";
		bodySize.unit = 'K';
		return bodySize;
	}
	number = value.substr(0, value.find_first_not_of("0123456789"));
	if (number.empty() || number.at(0) == '\0')
		throw std::runtime_error("client_max_body_size no number found");
	unit = value.substr(value.find_first_not_of("0123456789"));
	if (unit.size() != 1)
		throw std::runtime_error("client_max_body_size invalid unit size");
	if (unit.at(0) == 'K' || unit.at(0) == 'k')
		unit = "K";
	else if (unit.at(0) == 'M' || unit.at(0) == 'm')
		unit = "M";
	else if (unit.at(0) == 'G' || unit.at(0) == 'g')
		unit = "G";
	else
		throw std::runtime_error("client_max_body_size invalid unit (k/K, m/M, g/G)");
	bodySize.value = number;
	bodySize.unit = unit;
	return bodySize;
}

static bool	isHttpCode(std::string const &code)
{
	if (code.size() != 3)
		return false;
	if (code.find_first_not_of("0123456789") != std::string::npos)
		return false;
	// maybe check if it is in a range of http codes
	return true;
}

static void	setClientMaxBodySize(BodySize &clientMaxBodySize, int &bodySize, std::vector<std::string> split)
{
	std::stringstream	ss;

	if (split.size() != 1)
		throw std::runtime_error("client max body size usage : {SIZE}[UNIT] (42M or 300K or 9G)");
	clientMaxBodySize = createBodySize(split[0]);
	ss << clientMaxBodySize.value;
	ss >> bodySize;
	if (clientMaxBodySize.unit == "K")
		bodySize *= 1024;
	else if (clientMaxBodySize.unit == "M")
		bodySize *= 1024 * 1024;
	else if (clientMaxBodySize.unit == "G")
		bodySize *= 1024 * 1024 * 1024;
}

static void	setAllowedMethods(std::vector<http_method> &methods, std::vector<std::string> split)
{
	if (split.size() < 1)
		throw std::runtime_error("allowed_methos directive usage : allowed_methods GET|POST");
	// WORK IN PROGRESS
	methods.push_back(GET);
	methods.push_back(POST);
	methods.push_back(DELETE);
}

static void	setCgi(std::map<std::string, std::string> &cgiParams, std::vector<std::string> split)
{
	if (split.size() != 2)
		throw std::runtime_error("cgi directive must have an extension and a file");
	if (split[0] != ".py" && split[0] != ".php" && split[0] != ".pl" && split[0] != ".cgi")
		throw std::runtime_error("cgi extension must be .py, .php, .pl or .cgi");
	if (cgiParams.find(split[0]) == cgiParams.end())
		cgiParams[split[0]] = split[1];
	else
		throw std::runtime_error("double cgi directive for the same extension : " + split[0]);
}

static void	setReturn(std::map<std::string, std::string> &redirects, bool &redirection, std::vector<std::string> split)
{
	if (split.size() != 2)
		throw std::runtime_error("return directive must have a code and an uri");
	if (!isHttpCode(split[0]))
		throw std::runtime_error("return uri is missing at the end of the directive");
	if (redirects.find(split[0]) == redirects.end())
		redirects[split[0]] = split[split.size() - 1];
	redirection = true;
}

static void	setErrorPage(std::map<std::string, std::string> &errorPages, std::vector<std::string> split)
{
	if (split.size() < 2)
		throw std::runtime_error("error_page directive must have a code and an uri");
	if (isHttpCode(split[split.size() - 1]))
		throw std::runtime_error("error_page uri is missing at the end of the directive");
	for (size_t i = 0; (i < split.size() && isHttpCode(split[i])); i++)
	{
		if (errorPages.find(split[i]) == errorPages.end())
			errorPages[split[i]] = split[split.size() - 1];
	}
}

static std::vector<std::string>	stringSplit(std::string const &line)
{
	std::vector<std::string>	res;
	std::string					word;
	std::stringstream			ss(line);

	while (std::getline(ss, word, ' '))
	{
		if (!word.empty())
			res.push_back(word);
	}
	return res;
}

static bool	isServerBlock(std::string const &line)
{
	std::vector<std::string>	split = stringSplit(line);
	
	// if (split.size() != 2)
	// 	throw std::runtime_error("Server block usage : server {/*directives*/}");
	if (split[0] == "server" && split[1] == "{")
		return true;
	return false;
}

static bool	isLocationBlock(std::string const &line)
{
	std::vector<std::string>	split = stringSplit(line);

	if (line.find("location"))// if no location found
		return false;
	if (split.size() != 3)
		throw std::runtime_error("Location block usage : location PATH {/*directives*/}");
	if (split[0] == "location" && split[2] == "{")
		return true;
	return false;
}

static bool	isDirective(std::string const &line)
{
	std::stringstream	ss(line);
	std::string			word;
	std::string			dir[13] = {
		"listen", "server_name", "index", "root", 
		"client_max_body_size", "autoindex", "error_page", 
		"return", "cgi", "allowed_methods", "alias", 
		"path_info", "upload_location"
	};

	if (isServerBlock(line))
		throw std::runtime_error("A server block need to be closed before starting new one");
	if (line.at(line.size() - 1) != ';')
		throw std::runtime_error("Directive must end with a semicolon");
	ss >> word;
	for (int i = 0; i < 13; i++)
	{
		if (word == dir[i] || word == dir[i] + ";")
			return true;
	}
	return false;
}

static void	trimWhiteSpaces(std::string &str)
{
	str.erase(str.find_last_not_of(" \t") + 1);
	str.erase(0, str.find_first_not_of(" \t"));
}

static bool	isLineToIgnore(std::string line)
{
	if (line.empty())
		return true;
	if (line.find_first_not_of(" \t\r") == std::string::npos)
		return true;
	if (line.at(0) == '#')
		return true;
	return false;
}

static void	isOnOrOff(std::string const &key, std::string const &value, bool &toFill, std::vector<std::string> split)
{
	if (split.size() != 1)
		throw std::runtime_error("This directive '" + key + "' allow \"on\" or \"off\"");
	if (value != "on" && value != "off")
		throw std::runtime_error("Invalid value '" + value + "'");
	toFill = false;
	if (value == "on")
		toFill = true;
}

static void	initServerBlock(ServerBlock &serverBlock)
{
	serverBlock.port = 8080;
	serverBlock.host = "localhost";
	serverBlock.serverNames.clear();
	serverBlock.root = "./www";
	serverBlock.clientMaxBodySize.value = "0";
	serverBlock.clientMaxBodySize.unit = "G";
	serverBlock.bodySize = 0;
	serverBlock.autoindex = false;
	serverBlock.indexes.clear();
	serverBlock.errorPages.clear();
	serverBlock.redirects.clear();
	serverBlock.redirection = false;
	serverBlock.cgiParams.clear();
	serverBlock.methods.clear();
	serverBlock.locationBlocks.clear();
}

static void	initLocationBlock(LocationBlock &locationBlock)
{
	locationBlock.path = "";
	locationBlock.alias = "";
	locationBlock.root = "";
	locationBlock.clientMaxBodySize.value = "";
	locationBlock.clientMaxBodySize.unit = "";
	locationBlock.bodySize = 0;
	locationBlock.autoindex = false;
	locationBlock.pathInfo = false;
	locationBlock.indexes.clear();
	locationBlock.errorPages.clear();
	locationBlock.uploadLocation = "";
	locationBlock.redirects.clear();
	locationBlock.cgiParams.clear();
	locationBlock.methods.clear();
}

static void	makeAllLocationBlocksHeritate(std::vector<ServerBlock> &m_serverBlocks)
{
	for (std::vector<ServerBlock>::iterator it = m_serverBlocks.begin(); it != m_serverBlocks.end(); ++it)
	{
		for (std::vector<LocationBlock>::iterator it2 = it->locationBlocks.begin(); it2 != it->locationBlocks.end(); ++it2)
		{
			if (it2->root.empty())
				it2->root = it->root;
			if (it2->clientMaxBodySize.value.empty())
			{
				it2->clientMaxBodySize.value = it->clientMaxBodySize.value;
				it2->clientMaxBodySize.unit = it->clientMaxBodySize.unit;
				it2->bodySize = it->bodySize;
			}
			if (it2->indexes.empty())
				it2->indexes.push_back("default.html");
			if (it2->errorPages.empty())
				it2->errorPages = it->errorPages;
			if (it2->errorPages.empty())
				it2->errorPages = it->errorPages;
		}
	}
}

static void	setListen(std::string const &value, ServerBlock &serverBlock, std::vector<std::string> split)
{
	std::stringstream	ss;
	std::string 		port;

	if (split.size() != 0)
		throw std::runtime_error("Listen directive need only one value");
	if (value.empty() || value.find(' ') != std::string::npos)
		throw std::runtime_error("Invalid value'" + value + "'");
	if (value.at(0) == ':')
		throw std::runtime_error("wrong format for listen directive");
	if (value.find_first_of(':') != std::string::npos)
	{
		if (value.find_first_of(':') != value.find_last_of(':'))
			throw std::runtime_error("Invalid value '" + value + "'");
	}
	if (value.find_first_of(':') != std::string::npos)
	{
		serverBlock.host = value.substr(0, value.find(':'));
		ss << value.substr(value.find(':') + 1);
		if (ss.str().find_first_not_of("0123456789") != std::string::npos)
			throw std::runtime_error("Port must be a number.");
		ss >> serverBlock.port;
	}
	else
	{
		ss << value;
		if (ss.str().find_first_not_of("0123456789") != std::string::npos)
			throw std::runtime_error("Port must be a number.");
		ss >> serverBlock.port;
	}
}

static void	setStringValue(std::string const &key, std::string &valueToSet, std::vector<std::string> split)
{
	if (split.size() != 1)
		throw std::runtime_error("This directive '" + key + "' need only one value");
	valueToSet = split[0];
}

void	Configuration::parseLocationDirective(std::string &line, LocationBlock &locationBlock)
{
	std::vector<std::string>	split;
	std::string					str = line;
	std::string					key;

	if (str.at(str.size() - 1) != ';')
		throw std::runtime_error("Directive '" + str + "' must end with a semicolon");
	str.erase(str.size() - 1);
	split = stringSplit(str);
	if (split.size() == 0)
		throw std::runtime_error("Invalid directive '" + str + "'");
	key = split[0];
	split.erase(split.begin());
	if (key == "alias")
		setStringValue(key, locationBlock.alias, split);
	else if (key == "root")
		setStringValue(key, locationBlock.root, split);
	else if (key == "client_max_body_size")
		setClientMaxBodySize(locationBlock.clientMaxBodySize, locationBlock.bodySize, split);
	else if (key == "autoindex")
		isOnOrOff(key, split[0], locationBlock.autoindex, split);
	else if (key == "path_info")
		isOnOrOff(key, split[0], locationBlock.pathInfo, split);
	else if (key == "index")
		locationBlock.indexes = split;
	else if (key == "error_page")
		setErrorPage(locationBlock.errorPages, split);
	else if (key == "upload_location")
		setStringValue(key, locationBlock.uploadLocation, split);
	else if (key == "return")
		setReturn(locationBlock.redirects, locationBlock.redirection, split);
	else if (key == "cgi")
		setCgi(locationBlock.cgiParams, split);
	else if (key == "allowed_methods")
		setAllowedMethods(locationBlock.methods, split);
	else
		throw std::runtime_error("Unknown directive '" + key + "'");
}

void	Configuration::parseLocationBlock(ServerBlock &serverBlock, std::string const &locationLine, std::stringstream &ss)
{
	std::vector<std::string>	split = stringSplit(locationLine);
	LocationBlock				locationBlock;
	std::string					line;

	// std::cout << "LOCATION BLOCK" << std::endl;
	initLocationBlock(locationBlock);
	locationBlock.path = split[1];
	while (std::getline(ss, line))
	{
		trimWhiteSpaces(line);
		if (line.empty() || line.at(0) == '#')
			continue;
		else if (line == "}")
			break;
		else
		{
			parseLocationDirective(line, locationBlock);
			// std::cout << "line [" << line << "] is a location directive" << std::endl;
		}
	}
	serverBlock.locationBlocks.push_back(locationBlock);
	// std::cout << "-----------------" << std::endl;
}

void	Configuration::parseServerDirective(std::string const &line, ServerBlock &serverBlock)
{
	std::vector<std::string>	split;
	std::string					str = line;
	std::string					key;

	if (str.at(str.size() - 1) != ';')
		throw std::runtime_error("Directive '" + str + "' must end with a semicolon");
	str.erase(str.size() - 1);
	split = stringSplit(str);
	if (split.size() == 0)
		throw std::runtime_error("Invalid directive '" + str + "'");
	// std::cout << "str : [" << str << "]" << std::endl;
	key = split[0];
	// remove split[0] from split
	split.erase(split.begin());
	// std::cout << "split size : " << split.size() << std::endl;
	
	if (key == "autoindex")
		isOnOrOff(key, split[0], serverBlock.autoindex, split);
	else if (key == "root")
		setStringValue(key, serverBlock.root, split);
	else if (key == "client_max_body_size")
		setClientMaxBodySize(serverBlock.clientMaxBodySize, serverBlock.bodySize, split);
	else if (key == "listen")
		setListen(split[0], serverBlock, split);
	else if (key == "server_name")
		serverBlock.serverNames = split;
	else if (key == "index")
		serverBlock.indexes = split;
	else if (key == "error_page")
		setErrorPage(serverBlock.errorPages, split);
	else if (key == "return")
		setReturn(serverBlock.redirects, serverBlock.redirection, split);
	else if (key == "cgi")
		setCgi(serverBlock.cgiParams, split);
	else if (key == "allowed_methods")
		setAllowedMethods(serverBlock.methods, split);
	else
		throw std::runtime_error("Unknown directive '" + key + "'");
}

static void	pushServerBlock(std::vector<ServerBlock> &serverBlocks, ServerBlock &serverBlock)
{
	if (serverBlock.serverNames.empty())
		serverBlock.serverNames.push_back("webserv");
	if (serverBlock.locationBlocks.empty())
	{
		LocationBlock	locationBlock;

		initLocationBlock(locationBlock);
		locationBlock.root = serverBlock.root;
		locationBlock.errorPages = serverBlock.errorPages;
		locationBlock.clientMaxBodySize.value = serverBlock.clientMaxBodySize.value;
		locationBlock.clientMaxBodySize.unit = serverBlock.clientMaxBodySize.unit;
		serverBlock.locationBlocks.push_back(locationBlock);
	}
	serverBlocks.push_back(serverBlock);
}

void	Configuration::parseServerBlock(std::stringstream &ss)
{
	ServerBlock	server;
	std::string	line;

	// std::cout << "SERVER BLOCK" << std::endl;
	curlyBrackets++;
	initServerBlock(server);
	while (std::getline(ss, line))
	{
		trimWhiteSpaces(line);
		if (line.empty() || line.at(0) == '#')
			continue;
		else if (isServerBlock(line))
			throw std::runtime_error("No server blocks are allowed inside another server block");
		else if (line == "}")
			break;
		else if (isLocationBlock(line))
		{
			// std::cout << "line [" << line << "] is a location block" << std::endl;
			parseLocationBlock(server, line, ss);
		}
		else if (isDirective(line))
		{
			// std::cout << "line [" << line << "] is a server directive" << std::endl;
			parseServerDirective(line, server);
		}
		else
			throw std::runtime_error("Unknown directive at this line : [" + line + "]");
	}
	pushServerBlock(m_serverBlocks, server);
	curlyBrackets--;
	// std::cout << std::endl << std::endl;
}


Configuration::Configuration()
{
	ServerBlock	server;

	initServerBlock(server);
	pushServerBlock(m_serverBlocks, server);
}

Configuration::Configuration(std::string const &t_configFile) : m_configFile(t_configFile), curlyBrackets(0)
{
	std::ifstream		file(m_configFile.c_str());
	std::string			line;
	std::stringstream	ss;

	if (!file)
		throw std::runtime_error("Cannot open file " + m_configFile);
	ss << file.rdbuf();
	file.close();
	while (std::getline(ss, line))
	{
		trimWhiteSpaces(line);
		if (line.empty() || line.at(0) == '#')
			continue;
		else if (!isServerBlock(line))
			throw std::runtime_error("You need to start with a server block instead of '" + line + "'");
		if (curlyBrackets != 0)
			throw std::runtime_error("A server block need to be closed before opening a new one");
		// std::cout << "line [" << line << "] is a server block" << std::endl;
		parseServerBlock(ss);
	}
	if (m_serverBlocks.empty())
	{
		// OR : No server block found so let's launch the default one
		throw std::runtime_error("No server block found");
	}

	/*
	this make heritate every location variables with 
	server values (server variables down to location blocks)
	*/
	makeAllLocationBlocksHeritate(m_serverBlocks);
}

Configuration::~Configuration()
{
}

std::vector<ServerBlock> const &Configuration::getServerBlocks() const
{
	return m_serverBlocks;
}

const int	Configuration::getBodySize(BodySize const &bodySize)
{
	std::stringstream	ss;
	int					weight;

	ss << bodySize.value;
	ss >> weight;
	if (bodySize.unit == "K")
		weight *= 1024;
	else if (bodySize.unit == "M")
		weight *= 1024 * 1024;
	else if (bodySize.unit == "G")
		weight *= 1024 * 1024 * 1024;
	return weight;
}

std::vector<int>	Configuration::getPorts() const
{
	std::vector<int>	ports;

	for (std::vector<ServerBlock>::const_iterator it = m_serverBlocks.begin(); it != m_serverBlocks.end(); ++it)
		ports.push_back(it->port);
	return ports;
}

void	Configuration::printConfig() const
{
	for(std::vector<ServerBlock>::const_iterator it = m_serverBlocks.begin(); it != m_serverBlocks.end(); ++it)
	{
		std::cout << "\033[0;33;42m----- SERVER -----\033[0m" << std::endl;
		
		// LISTEN
		std::cout << "port: " << it->port << std::endl;
		std::cout << "host: " << it->host << std::endl;
		
		// SERVER NAMES
		std::cout << "serverNames: ";
		for (std::vector<std::string>::const_iterator it2 = it->serverNames.begin(); it2 != it->serverNames.end(); ++it2)
			std::cout << *it2 << " ";
		std::cout << std::endl;
		
		// ROOT
		std::cout << "root: " << it->root << std::endl;
		
		// ERROR PAGES
		std::cout << "errorPages: ";
		for (std::map<std::string, std::string>::const_iterator it2 = it->errorPages.begin(); it2 != it->errorPages.end(); ++it2)
			std::cout << it2->first << ":" << it2->second << " ";
		std::cout << std::endl;
		
		// CLIENT MAX BODY SIZE
		std::cout << "clientMaxBodySize: " << it->clientMaxBodySize.value << " " << it->clientMaxBodySize.unit << std::endl;
		std::cout << "bodySize: " << it->bodySize << std::endl;
		
		// AUTOINDEX
		std::cout << "autoindex: " << (it->autoindex ? "on" : "off") << std::endl;

		// INDEXES
		std::cout << "indexes: ";
		for (std::vector<std::string>::const_iterator it2 = it->indexes.begin(); it2 != it->indexes.end(); ++it2)
			std::cout << *it2 << " ";
		std::cout << std::endl;

		// ERROR PAGES
		std::cout << "errorPages: ";
		for (std::map<std::string, std::string>::const_iterator it2 = it->errorPages.begin(); it2 != it->errorPages.end(); ++it2)
			std::cout << it2->first << ":" << it2->second << " ";
		std::cout << std::endl;

		// REDIRECTS
		std::cout << "redirection : " << (it->redirection ? "on" : "off") << std::endl;
		std::cout << "redirects: ";
		for (std::map<std::string, std::string>::const_iterator it2 = it->redirects.begin(); it2 != it->redirects.end(); ++it2)
			std::cout << it2->first << " " << it2->second << " ";
		std::cout << std::endl;

		// CGI PARAMS
		std::cout << "cgiParams: ";
		for (std::map<std::string, std::string>::const_iterator it2 = it->cgiParams.begin(); it2 != it->cgiParams.end(); ++it2)
			std::cout << it2->first << " " << it2->second << " ";
		std::cout << std::endl;

		// METHODS
		std::cout << "methods: ";
		for (std::vector<http_method>::const_iterator it2 = it->methods.begin(); it2 != it-> methods.end(); ++it2)
			std::cout << *it2 << " ";
		std::cout << std::endl;

		// LOCATION BLOCKS
		for (std::vector<LocationBlock>::const_iterator it2 = it->locationBlocks.begin(); it2 != it->locationBlocks.end(); ++it2)
		{
			std::cout << "--LOCATION--" << std::endl;
			std::cout << "  path: " << it2->path << std::endl;
			std::cout << "  alias: " << it2->alias << std::endl;
			std::cout << "  root: " << it2->root << std::endl;
			std::cout << "  clientMaxBodySize: " << it2->clientMaxBodySize.value << " " << it2->clientMaxBodySize.unit << std::endl;
			std::cout << "  body size: " << it2->bodySize << std::endl;
			std::cout << "  autoindex: " << (it2->autoindex ? "on" : "off") << std::endl;
			std::cout << "  pathInfo: " << (it2->pathInfo ? "on" : "off") << std::endl;

			std::cout << "  indexes: ";
			for (std::vector<std::string>::const_iterator it3 = it2->indexes.begin(); it3 != it2->indexes.end(); ++it3)
				std::cout << *it3 << " ";
			std::cout << std::endl;
			
			std::cout << "  errorPages: ";
			for (std::map<std::string, std::string>::const_iterator it3 = it2->errorPages.begin(); it3 != it2->errorPages.end(); ++it3)
				std::cout << it3->first << ":" << it3->second << " ";
			std::cout << std::endl;
			
			std::cout << "  uploadLocation: " << it2->uploadLocation << std::endl;

			std::cout << "  redirects: ";
			for (std::map<std::string, std::string>::const_iterator it3 = it2->redirects.begin(); it3 != it2->redirects.end(); ++it3)
				std::cout << it3->first << " " << it3->second << " ";
			std::cout << std::endl;

			std::cout << "  redirection: " << (it2->redirection ? "true" : "false") << std::endl;

			std::cout << "  cgiParams: ";
			for (std::map<std::string, std::string>::const_iterator it3 = it2->cgiParams.begin(); it3 != it2->cgiParams.end(); ++it3)
				std::cout << it3->first << " " << it3->second << " ";
			std::cout << std::endl;

			std::cout << "  methods (" << it2->methods.size() << "): ";
			for (std::vector<http_method>::const_iterator it3 = it2->methods.begin(); it3 != it2-> methods.end(); ++it3)
				std::cout << *it3 << " ";
			std::cout << std::endl;
		}
		std::cout << "\033[0;33;42m------------------\033[0m" << std::endl;
	}
}





// static void	checkDirectivePairsWithSingleValue(std::string const &key, std::string const &value)
// {
// 	std::string	keysWithOneValue[7] = {
// 		"listen", "root", "alias", "client_max_body_size", 
// 		"autoindex", "path_info", "upload_location"
// 	};
	
// 	for (int i = 0; i < 7; i++)
// 	{
// 		if (key == keysWithOneValue[i])
// 		{
// 			if (value.find_first_of(" \t\n\v\f\r") != std::string::npos)
// 				throw std::runtime_error("Directive '" + key + "' must have only one value");
// 		}
// 	}
// }

// static void	addUniqueNewValueToMap(std::map<std::string, std::string> &map, std::string const &key, std::string const &value)
// {
// 	if (map.find(key) != map.end())
// 		throw std::runtime_error("double error_page directive for the same code : " + key);
// 	map[key] = value;
// }


// static void	setMethod(std::string const &value, LocationBlock &locationBlock)
// {
// 	std::string	split;
// 	std::stringstream	ss(value);
// 	int			get = 0;
// 	int			post = 0;
// 	int			del = 0;

// 	while(std::getline(ss, split, '|'))
// 	{
// 		if (split == "GET" && get == 0)
// 		{
// 			locationBlock.methods.push_back(GET);
// 			get++;
// 		}
// 		else if (split == "POST" && post == 0)
// 		{
// 			locationBlock.methods.push_back(POST);
// 			post++;
// 		}
// 		else if (split == "DELETE" && del == 0)
// 		{
// 			locationBlock.methods.push_back(DELETE);
// 			del++;
// 		}
// 		else
// 			throw std::runtime_error("Method directive not conform");
// 	}
// 	if (locationBlock.methods.empty())
// 	{
// 		// locationBlock.methods.push_back(GET);
// 		// locationBlock.methods.push_back(POST);
// 		// locationBlock.methods.push_back(DELETE);
// 	}
// }

// static void	parseListOfWords(std::string const &value, std::vector<std::string> &names)
// {
// 	std::istringstream	iss(value);
// 	std::string			word;

// 	while (iss >> word && !word.empty())
// 		names.push_back(word);
// }

// void	Configuration::setServerValues(std::string const &key, std::string const &value, ServerBlock &serverBlock)
// {
// 	std::string	code;

// 	checkDirectivePairsWithSingleValue(key, value);
// 	if (key == "listen")
// 		setListen(value, serverBlock);
// 	else if (key == "server_name")
// 		parseListOfWords(value, serverBlock.serverNames);
// 	else if (key == "root")
// 		serverBlock.root = value;
// 	else if (key == "error_page")
// 	{
// 		code = getKey(value);
// 		if (code.find_first_not_of("0123456789") != std::string::npos)
// 			throw std::runtime_error("error_page code must be a number");
// 		if (serverBlock.errorPages.find(key) == serverBlock.errorPages.end())
// 			serverBlock.errorPages[code] = getValue(value);
// 	}
// 	else if (key == "client_max_body_size")
// 	{
// 		serverBlock.clientMaxBodySize = createBodySize(value);
// 		serverBlock.bodySize = getBodySize(serverBlock.clientMaxBodySize);
// 	}
// 	else
// 		throw std::runtime_error("Unknown directive '" + key + "'");
// }

// void	Configuration::setLocationValues(std::string const &key, std::string const &value, LocationBlock &locationBlock)
// {
// 	checkDirectivePairsWithSingleValue(key, value);
// 	if (key == "root")
// 		locationBlock.root = value;
// 	else if (key == "alias")
// 		locationBlock.alias = value;
// 	else if (key == "client_max_body_size")
// 	{
// 		locationBlock.clientMaxBodySize = createBodySize(value);
// 		locationBlock.bodySize = getBodySize(locationBlock.clientMaxBodySize);
// 	}
// 	else if (key == "autoindex")
// 		locationBlock.autoindex = isOnOrOff(value);
// 	else if (key == "index")
// 		parseListOfWords(value, locationBlock.indexes);
// 	else if (key == "return")
// 	{
// 		std::string	code = getKey(value);
// 		if (code.find_first_not_of("0123456789") != std::string::npos)
// 			throw std::runtime_error("return code must be a number");
// 		addUniqueNewValueToMap(locationBlock.redirects, code, getValue(value));
// 	}
// 	else if (key == "path_info")
// 		locationBlock.pathInfo = isOnOrOff(value);
// 	else if (key == "cgi")
// 		addUniqueNewValueToMap(locationBlock.cgiParams, getKey(value), getValue(value));
// 	else if (key == "upload_location")
// 		locationBlock.uploadLocation = value;
// 	else if (key == "error_page")
// 	{
// 		std::string	code = getKey(value);
// 		if (code.find_first_not_of("0123456789") != std::string::npos)
// 			throw std::runtime_error("error_page code must be a number");
// 		if (locationBlock.errorPages.find(code) == locationBlock.errorPages.end())
// 			locationBlock.errorPages[code] = getValue(value);
// 		// addUniqueNewValueToMap(locationBlock.errorPages, code, getValue(value));
// 	}
// 	else if (key == "allowed_methods")
// 		setMethod(value, locationBlock);
// 	else
// 		throw std::runtime_error("[setLocationValues]Unknown directive '" + key + "'");
// }


// static std::string	getKey(std::string const &value)
// {
// 	std::string	key;

// 	key = value.substr(0, value.find_first_of(" \t"));
// 	return key;
// }

// static std::string	getValue(std::string const &value)
// {
// 	std::string	val;

// 	val = value.substr(value.find_first_not_of(" \t", getKey(value).size()));
// 	if (val.empty() || val.at(0) == '\0')
// 		throw std::runtime_error("error_page uri is empty.");
// 	if (val.find_first_of(" \t") != std::string::npos)
// 		throw std::runtime_error("error_page uri must be a single value.");
// 	return val;
// }