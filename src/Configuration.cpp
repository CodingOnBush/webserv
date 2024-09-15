#include "../include/Configuration.hpp"
#include "Configuration.hpp"

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
	
	if (split.size() != 2)
		return false;
	if (split[0] == "server" && split[1] == "{")
		return true;
	return false;
}

static bool	isLocationBlock(std::string const &line)
{
	std::vector<std::string>	split = stringSplit(line);

	if (split.size() != 3)
		return false;
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
		// std::cout << "dir[i] = " << dir[i] << std::endl;
		if (word == dir[i] || word == dir[i] + ";")
		{
			// std::cout << "YES" << std::endl;
			return true;
		}
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

static bool	isOnOrOff(std::string const &value)
{
	if (value != "on" && value != "off")
		throw std::runtime_error("Invalid value '" + value + "'");
	return (value == "on");
}

static void	checkDirectivePairsWithSingleValue(std::string const &key, std::string const &value)
{
	std::string	keysWithOneValue[7] = {
		"listen", "root", "alias", "client_max_body_size", 
		"autoindex", "path_info", "upload_location"
	};
	
	for (int i = 0; i < 7; i++)
	{
		if (key == keysWithOneValue[i])
		{
			if (value.find_first_of(" \t\n\v\f\r") != std::string::npos)
				throw std::runtime_error("Directive '" + key + "' must have only one value");
		}
	}
}

static void	addUniqueNewValueToMap(std::map<std::string, std::string> &map, std::string const &key, std::string const &value)
{
	if (map.find(key) != map.end())
		throw std::runtime_error("double error_page directive for the same code : " + key);
	map[key] = value;
}

static void	initLocationBlock(LocationBlock &locationBlock)
{
	locationBlock.path = "/";
	locationBlock.root = "";
	locationBlock.alias = "";
	locationBlock.clientMaxBodySize.value = "";
	locationBlock.clientMaxBodySize.unit = "";
	locationBlock.autoindex = false;
	locationBlock.indexes.clear();
	locationBlock.redirects.clear();
	locationBlock.pathInfo = false;
	locationBlock.cgiParams.clear();
	locationBlock.uploadLocation = "";
	locationBlock.errorPages.clear();
	locationBlock.methods.clear();
}

static void	initServerBlock(ServerBlock &serverBlock)
{
	serverBlock.port = 8080;
	serverBlock.host = "localhost";
	serverBlock.serverNames.clear();
	serverBlock.root = "./www";
	serverBlock.errorPages.clear();
	serverBlock.clientMaxBodySize.value = "1";
	serverBlock.clientMaxBodySize.unit = "M";
	serverBlock.locationBlocks.clear();
}



static void	parseAllLocationBlocks(std::vector<ServerBlock> &m_serverBlocks)
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
			}
			//maybe will be changed
			if (it2->indexes.empty())
				it2->indexes.push_back("default.html");
			if (it2->errorPages.empty())
				it2->errorPages = it->errorPages;
			// if (it2->methods.empty())
			// {
			// 	it2->methods.push_back(GET);
			// 	it2->methods.push_back(POST);
			// 	it2->methods.push_back(DELETE);
			// }
			if (it2->errorPages.empty())
				it2->errorPages = it->errorPages;
		}
	}
}



static std::string	getLocationPath(std::string const &line)
{
	std::string res = line;

	if (res.empty())
		throw std::runtime_error("Location block must end with a '{'");
	res.erase(res.find_last_not_of(" \t") + 1);
	if (res.at(res.size() - 1) != '{')
		throw std::runtime_error("Location block must end with a '{'");
	if (res.at(0) == '{')
		throw std::runtime_error("Location block need a space before the '{'");
	res.erase(res.size() - 1);
	res.erase(0, res.find_first_not_of(" \t"));
	res.erase(res.find_last_not_of(" \t") + 1);
	if (res.empty())
		throw std::runtime_error("Each location block need a path");
	return res;
}

static std::string	getKey(std::string const &value)
{
	std::string	key;

	key = value.substr(0, value.find_first_of(" \t"));
	return key;
}

static std::string	getValue(std::string const &value)
{
	std::string	val;

	val = value.substr(value.find_first_not_of(" \t", getKey(value).size()));
	if (val.empty() || val.at(0) == '\0')
		throw std::runtime_error("error_page uri is empty.");
	if (val.find_first_of(" \t") != std::string::npos)
		throw std::runtime_error("error_page uri must be a single value.");
	return val;
}

static BodySize	createBodySize(std::string const &value)
{
	BodySize	bodySize;
	std::string	unit;
	std::string	number;

	if (value.empty() || value.at(0) == '\0')
		throw std::runtime_error("client_max_body_size no value");
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

static void	setListen(std::string const &value, ServerBlock &serverBlock)
{
	std::stringstream	ss;
	std::string 		port;

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

static void	setMethod(std::string const &value, LocationBlock &locationBlock)
{
	std::string	split;
	std::stringstream	ss(value);
	int			get = 0;
	int			post = 0;
	int			del = 0;

	while(std::getline(ss, split, '|'))
	{
		if (split == "GET" && get == 0)
		{
			locationBlock.methods.push_back(GET);
			get++;
		}
		else if (split == "POST" && post == 0)
		{
			locationBlock.methods.push_back(POST);
			post++;
		}
		else if (split == "DELETE" && del == 0)
		{
			locationBlock.methods.push_back(DELETE);
			del++;
		}
		else
			throw std::runtime_error("Method directive not conform");
	}
	if (locationBlock.methods.empty())
	{
		// locationBlock.methods.push_back(GET);
		// locationBlock.methods.push_back(POST);
		// locationBlock.methods.push_back(DELETE);
	}
}

static void	parseListOfWords(std::string const &value, std::vector<std::string> &names)
{
	std::istringstream	iss(value);
	std::string			word;

	while (iss >> word && !word.empty())
		names.push_back(word);
}

void	Configuration::setServerValues(std::string const &key, std::string const &value, ServerBlock &serverBlock)
{
	std::string	code;

	checkDirectivePairsWithSingleValue(key, value);
	if (key == "listen")
		setListen(value, serverBlock);
	else if (key == "server_name")
		parseListOfWords(value, serverBlock.serverNames);
	else if (key == "root")
		serverBlock.root = value;
	else if (key == "error_page")
	{
		code = getKey(value);
		if (code.find_first_not_of("0123456789") != std::string::npos)
			throw std::runtime_error("error_page code must be a number");
		if (serverBlock.errorPages.find(key) == serverBlock.errorPages.end())
			serverBlock.errorPages[code] = getValue(value);
	}
	else if (key == "client_max_body_size")
	{
		serverBlock.clientMaxBodySize = createBodySize(value);
		serverBlock.bodySize = getBodySize(serverBlock.clientMaxBodySize);
	}
	else
		throw std::runtime_error("Unknown directive '" + key + "'");
}

void	Configuration::setLocationValues(std::string const &key, std::string const &value, LocationBlock &locationBlock)
{
	checkDirectivePairsWithSingleValue(key, value);
	if (key == "root")
		locationBlock.root = value;
	else if (key == "alias")
		locationBlock.alias = value;
	else if (key == "client_max_body_size")
	{
		locationBlock.clientMaxBodySize = createBodySize(value);
		locationBlock.bodySize = getBodySize(locationBlock.clientMaxBodySize);
	}
	else if (key == "autoindex")
		locationBlock.autoindex = isOnOrOff(value);
	else if (key == "index")
		parseListOfWords(value, locationBlock.indexes);
	else if (key == "return")
	{
		std::string	code = getKey(value);
		if (code.find_first_not_of("0123456789") != std::string::npos)
			throw std::runtime_error("return code must be a number");
		addUniqueNewValueToMap(locationBlock.redirects, code, getValue(value));
	}
	else if (key == "path_info")
		locationBlock.pathInfo = isOnOrOff(value);
	else if (key == "cgi")
		addUniqueNewValueToMap(locationBlock.cgiParams, getKey(value), getValue(value));
	else if (key == "upload_location")
		locationBlock.uploadLocation = value;
	else if (key == "error_page")
	{
		std::string	code = getKey(value);
		if (code.find_first_not_of("0123456789") != std::string::npos)
			throw std::runtime_error("error_page code must be a number");
		if (locationBlock.errorPages.find(code) == locationBlock.errorPages.end())
			locationBlock.errorPages[code] = getValue(value);
		// addUniqueNewValueToMap(locationBlock.errorPages, code, getValue(value));
	}
	else if (key == "allowed_methods")
		setMethod(value, locationBlock);
	else
		throw std::runtime_error("[setLocationValues]Unknown directive '" + key + "'");
}

void	Configuration::parseLocationDirective(std::string &line, LocationBlock &locationBlock)
{
	// std::string	keys[11] = {
	// 	"root", "alias", "client_max_body_size", 
	// 	"autoindex", "index", "return", "path_info", 
	// 	"cgi", "upload_location", "allowed_methods", "error_page"
	// };
	std::vector<std::string>	split = stringSplit(line);

	if (isServerBlock(line))
		throw std::runtime_error("A server block need to be closed before starting new one");
	if (line.at(line.size() - 1) != ';')
		throw std::runtime_error("Directive '" + line + "' must end with a semicolon");
	line.erase(line.size() - 1);
	std::cout << "split[0] = " << split[0] << std::endl;
	// for (int i = 0; i < 11; i++)
	// {
	// 	if (split[0] == keys[i] || split[0] == keys[i] + ";")
	// 	{
	// 		std::cout << "." << std::endl;
	// 		return;
	// 	}
	// }
	
	// throw std::runtime_error("Unknown directive '" + split[0] + "'");
}

static void	pushLocationBlock(std::vector<LocationBlock> &locationBlocks, LocationBlock &locationBlock)
{
	if (locationBlock.root.empty())
		locationBlock.root = "./www";
	if (locationBlock.clientMaxBodySize.value.empty())
	{
		locationBlock.clientMaxBodySize.value = "1";
		locationBlock.clientMaxBodySize.unit = "M";
	}
	if (locationBlock.indexes.empty())
		locationBlock.indexes.push_back("default.html");
	locationBlocks.push_back(locationBlock);
}

void	Configuration::parseLocationBlock(ServerBlock &serverBlock, std::string const &locationLine, std::stringstream &ss)
{
	std::vector<std::string>	split = stringSplit(locationLine);
	LocationBlock				locationBlock;
	std::string					line;

	std::cout << "LOCATION BLOCK" << std::endl;
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
			std::cout << "line [" << line << "] is a location directive" << std::endl;
			parseLocationDirective(line, locationBlock);
		}
	}
	pushLocationBlock(serverBlock.locationBlocks, locationBlock);
	std::cout << "LOCATION BLOCK END" << std::endl;
}

void	Configuration::parseServerDirective(std::string const &line, ServerBlock &serverBlock)
{
	std::string					single[4] = {"listen", "root", "client_max_body_size", "autoindex"};
	std::string					listed[2] = {"server_name", "index"};
	std::string					pairs[2] = {"return", "cgi"};
	std::vector<std::string>	split;

	if (line.at(line.size() - 1) != ';')
		throw std::runtime_error("Directive '" + line + "' must end with a semicolon");
	split = stringSplit(line);
	if (split.size() < 1)
		throw std::runtime_error("Invalid directive '" + line + "'");

	// throw std::runtime_error("Unknown directive '" + split[0] + "'");
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

	std::cout << "SERVER BLOCK" << std::endl;
	curlyBrackets++;
	initServerBlock(server);
	while (std::getline(ss, line))
	{
		trimWhiteSpaces(line);
		if (line.empty() || line.at(0) == '#')
			continue;
		else if (line == "}")
			break;
		else if (isServerBlock(line))
			throw std::runtime_error("No server blocks are allowed inside another server block");
		else if (isLocationBlock(line))
		{
			std::cout << "line [" << line << "] is a location block" << std::endl;
			parseLocationBlock(server, line, ss);
		}
		else if (isDirective(line))
		{
			std::cout << "line [" << line << "] is a server directive" << std::endl;
			parseServerDirective(line, server);
		}
		else
			throw std::runtime_error("Unknown directive at this line : [" + line + "]");
	}
	pushServerBlock(m_serverBlocks, server);
	curlyBrackets--;
	std::cout << "SERVER BLOCK END" << std::endl << std::endl;
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
		std::cout << "line [" << line << "] is a server block" << std::endl;
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
	parseAllLocationBlocks(m_serverBlocks);
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
		std::cout << "port: " << it->port << std::endl;
		std::cout << "host: " << it->host << std::endl;
		std::cout << "serverNames: ";
		for (std::vector<std::string>::const_iterator it2 = it->serverNames.begin(); it2 != it->serverNames.end(); ++it2)
			std::cout << *it2 << " ";
		std::cout << std::endl;
		std::cout << "root: " << it->root << std::endl;
		std::cout << "errorPages: ";
		for (std::map<std::string, std::string>::const_iterator it2 = it->errorPages.begin(); it2 != it->errorPages.end(); ++it2)
			std::cout << it2->first << ":" << it2->second << " ";
		std::cout << std::endl;
		std::cout << "clientMaxBodySize: " << it->clientMaxBodySize.value << " " << it->clientMaxBodySize.unit << std::endl;
		for (std::vector<LocationBlock>::const_iterator it2 = it->locationBlocks.begin(); it2 != it->locationBlocks.end(); ++it2)
		{
			std::cout << "--LOCATION--" << std::endl;
			std::cout << "  path: " << it2->path << std::endl;
			std::cout << "  root: " << it2->root << std::endl;
			std::cout << "  alias: " << it2->alias << std::endl;
			std::cout << "  clientMaxBodySize: " << it2->clientMaxBodySize.value << " " << it2->clientMaxBodySize.unit << std::endl;
			std::cout << "  autoindex: " << (it2->autoindex ? "on" : "off") << std::endl;
			std::cout << "  indexes: ";
			for (std::vector<std::string>::const_iterator it3 = it2->indexes.begin(); it3 != it2->indexes.end(); ++it3)
				std::cout << *it3 << " ";
			std::cout << std::endl;
			std::cout << "  redirects: ";
			for (std::map<std::string, std::string>::const_iterator it3 = it2->redirects.begin(); it3 != it2->redirects.end(); ++it3)
				std::cout << it3->first << " " << it3->second << " ";
			std::cout << std::endl;
			std::cout << "  pathInfo: " << (it2->pathInfo ? "on" : "off") << std::endl;
			std::cout << "  cgiParams: ";
			for (std::map<std::string, std::string>::const_iterator it3 = it2->cgiParams.begin(); it3 != it2->cgiParams.end(); ++it3)
				std::cout << it3->first << " " << it3->second << " ";
			std::cout << std::endl;
			std::cout << "  uploadLocation: " << it2->uploadLocation << std::endl;
			std::cout << "  errorPages: ";
			for (std::map<std::string, std::string>::const_iterator it3 = it2->errorPages.begin(); it3 != it2->errorPages.end(); ++it3)
				std::cout << it3->first << ":" << it3->second << " ";
			std::cout << std::endl;
			std::cout << "  methods: ";
			for (std::vector<http_method>::const_iterator it3 = it2->methods.begin(); it3 != it2-> methods.end(); ++it3)
				std::cout << *it3 << " ";
			std::cout << std::endl;
		}
		std::cout << "\033[0;33;42m------------------\033[0m" << std::endl;
	}
}
