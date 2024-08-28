#include "../include/Configuration.hpp"

static void	initLocationBlock(LocationBlock &locationBlock)
{
	locationBlock.exactMatch = false;
	locationBlock.path = "";
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
	locationBlock.methods.clear();
}

static void	initServerBlock(ServerBlock &serverBlock)
{
	serverBlock.port = 8080;
	serverBlock.host = "localhost";
	serverBlock.serverNames.clear();
	serverBlock.root = "/";
	serverBlock.errorPages.clear();
	serverBlock.clientMaxBodySize.value = "10";
	serverBlock.clientMaxBodySize.unit = "M";
	serverBlock.locationBlocks.clear();
}

void	Configuration::initDirectiveMap()
{
	m_directives["listen"] = LISTEN;
	m_directives["server_name"] = SERVER_NAME;
	m_directives["root"] = ROOT;
	m_directives["error_page"] = ERROR_PAGE;
	m_directives["client_max_body_size"] = CLIENT_MAX_BODY_SIZE;
	m_directives["location"] = LOCATION;
	m_directives["autoindex"] = AUTOINDEX;
	m_directives["index"] = INDEX;
	m_directives["redirect"] = REDIRECT;
	m_directives["path_info"] = PATH_INFO;
	m_directives["cgi_param"] = CGI_PARAM;
	m_directives["upload_location"] = UPLOAD_LOCATION;
	m_directives["method"] = METHOD;
}

void	Configuration::setServerValues(std::string const &expression, std::string const &value, ServerBlock &serverBlock)
{
	if (value.rfind(";", 0) == 0)
		throw std::runtime_error("Directive '" + expression + "' must have a value");
	if (value[value.size() - 1] != ';')
		throw std::runtime_error("Directive '" + expression + "' must end with a semicolon");
	switch (m_directives[expression])
	{
	case ROOT:
		serverBlock.root = value;
		serverBlock.root[value.size() - 1] = '\0';
		break;
	
	default:
		throw std::runtime_error("[setServerValues]Unknown directive '" + expression + "'");
		break;
	}
}

void	Configuration::setLocationValues(std::string const &expression, std::string const &value, LocationBlock &locationBlock)
{
	if (value.rfind(";", 0) == 0)
		throw std::runtime_error("Directive '" + expression + "' must have a value");
	if (value[value.size() - 1] != ';')
		throw std::runtime_error("Directive '" + expression + "' must end with a semicolon");
	switch (m_directives[expression])
	{
	case ROOT:
		locationBlock.root = value;
		locationBlock.root[value.size() - 1] = '\0';
		break;
	
	default:
		throw std::runtime_error("[setLocationValues]Unknown directive '" + expression + "'");
		break;
	}
}

void	Configuration::parseServerDirective(std::string const &line, ServerBlock &serverBlock)
{
	std::string	directive;
	std::string	value;
	(void)serverBlock;
	if (line[line.size() - 1] != ';')
		throw std::runtime_error("Directive '" + line + "' must end with a semicolon");
	std::istringstream	iss(line);
	iss >> directive;
	iss >> value;
	setServerValues(directive, value, serverBlock);
}

void	Configuration::parseLocationDirective(std::string const &line, LocationBlock &locationBlock)
{
	std::string	directive;
	std::string	value;
	(void)locationBlock;
	if (line[line.size() - 1] != ';')
		throw std::runtime_error("Directive '" + line + "' must end with a semicolon");
	std::istringstream	iss(line);
	iss >> directive;
	iss >> value;
	// std::cout << "directive: {" << directive << "}";
	// std::cout << " value: {" << value << "}" << std::endl;
	setLocationValues(directive, value, locationBlock);
}

void	Configuration::parseLocationBlock(std::stringstream &content, ServerBlock &serverBlock)
{
	LocationBlock	locationBlock;
	std::string		line;

	std::cout << "location block" << std::endl;
	initLocationBlock(locationBlock);
	while (std::getline(content, line))
	{
		if (line.empty())
			continue;
		if (line == "\t}")
			break;
		else
			parseLocationDirective(line, locationBlock);
	}
	serverBlock.locationBlocks.push_back(locationBlock);
}

void	Configuration::parseServerBlock(std::stringstream &content)
{
	ServerBlock	server;
	std::string	line;

	std::cout << "server block" << std::endl;
	initServerBlock(server);
	while (std::getline(content, line))
	{
		if (line.empty())
			continue;
		if (line.rfind("\tlocation", 0) == 0)
			parseLocationBlock(content, server);
		else if (line == "}")
			break;
		else
			parseServerDirective(line, server);
	}
	m_serverBlocks.push_back(server);
}

void	Configuration::parseConfigFile()
{
	std::string	line;

	while (std::getline(m_content, line))
	{
		if (line.empty())
			continue;
		if (line == "server {")
			parseServerBlock(m_content);
		else
			throw std::runtime_error("[parseConfigFile]Unknown directive '" + line + "'");
	}
}

Configuration::Configuration(std::string const &t_configFile) : m_configFile(t_configFile)
{
	std::ifstream		file(m_configFile.c_str());

	if (!file)
		throw std::runtime_error("Cannot open file " + m_configFile);
	m_content << file.rdbuf();
	file.close();
	parseConfigFile();
}

Configuration::~Configuration()
{
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
			std::cout << it2->first << " " << it2->second << " ";
		std::cout << std::endl;
		std::cout << "clientMaxBodySize: " << it->clientMaxBodySize.value << " " << it->clientMaxBodySize.unit << std::endl;
		for (std::vector<LocationBlock>::const_iterator it2 = it->locationBlocks.begin(); it2 != it->locationBlocks.end(); ++it2)
		{
			std::cout << "  A location :" << std::endl;
			std::cout << "  exactMatch: " << it2->exactMatch << std::endl;
			std::cout << "  path: " << it2->path << std::endl;
			std::cout << "  root: " << it2->root << std::endl;
			std::cout << "  alias: " << it2->alias << std::endl;
			std::cout << "  clientMaxBodySize: " << it2->clientMaxBodySize.value << " " << it2->clientMaxBodySize.unit << std::endl;
			std::cout << "  autoindex: " << it2->autoindex << std::endl;
			std::cout << "  indexes: ";
			for (std::vector<std::string>::const_iterator it3 = it2->indexes.begin(); it3 != it2->indexes.end(); ++it3)
				std::cout << *it3 << " ";
			std::cout << std::endl;
			std::cout << "  redirects: ";
			for (std::map<std::string, std::string>::const_iterator it3 = it2->redirects.begin(); it3 != it2->redirects.end(); ++it3)
				std::cout << it3->first << " " << it3->second << " ";
			std::cout << std::endl;
			std::cout << "  pathInfo: " << it2->pathInfo << std::endl;
			std::cout << "  cgiParams: ";
			for (std::map<std::string, std::string>::const_iterator it3 = it2->cgiParams.begin(); it3 != it2->cgiParams.end(); ++it3)
				std::cout << it3->first << " " << it3->second << " ";
			std::cout << std::endl;
			std::cout << "  uploadLocation: " << it2->uploadLocation << std::endl;
			std::cout << "  methods: ";
			for (std::vector<http_method>::const_iterator it3 = it2->methods.begin(); it3 != it2-> methods.end(); ++it3)
				std::cout << *it3 << " ";
			std::cout << std::endl;
		}
		std::cout << "\033[0;33;42m------------------\033[0m" << std::endl;
	}
}
