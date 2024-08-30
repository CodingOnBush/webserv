#include "../include/Configuration.hpp"

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
	m_directives["return"] = RETURN;
	m_directives["path_info"] = PATH_INFO;
	m_directives["cgi"] = CGI;
	m_directives["alias"] = ALIAS;
	m_directives["upload_location"] = UPLOAD_LOCATION;
	m_directives["set_method"] = SET_METHOD;
}

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

std::string	Configuration::extractValue(std::string const &line)
{
	std::istringstream	iss(line);
	std::string			value;

	std::getline(iss, value);
	if (value[value.size() - 1] != ';')
		throw std::runtime_error("Directive '" + line + "' must end with a semicolon");
	value.erase(0, value.find_first_not_of(" \t"));
	value[value.size() - 1] = '\0';
	return (value);
}

std::string	Configuration::extractDirective(std::string const &line)
{
	std::istringstream	iss(line);
	std::string			directive;

	iss >> directive;
	directive.erase(0, directive.find_first_not_of(" \t"));
	return (directive);
}

void	Configuration::setServerValues(std::string const &expression, std::string const &value, ServerBlock &serverBlock)
{
	switch (m_directives[expression])
	{
	case LISTEN:
		std::cout << "listen: [" << value << "]" << std::endl;
		break;
	case SERVER_NAME:
		std::cout << "server_name: [" << value << "]" << std::endl;
		break;
	case ROOT:
		serverBlock.root = value;
		break;
	case ERROR_PAGE:
		std::cout << "error_page: [" << value << "]" << std::endl;
		break;
	case CLIENT_MAX_BODY_SIZE:
		std::cout << "client_max_body_size: [" << value << "]" << std::endl;
		break;
	default:
		throw std::runtime_error("[setServerValues]Unknown directive '" + expression + "'");
		break;
	}
}

void	Configuration::setLocationValues(std::string const &expression, std::string const &value, LocationBlock &locationBlock)
{
	switch (m_directives[expression])
	{
	case ROOT:
		locationBlock.root = value;
		// std::cout << "\troot: [" << value << "]" << std::endl;
		break;

	case ALIAS:
		locationBlock.alias = value;
		// std::cout << "\talias: [" << value << "]" << std::endl;
		break;

	case CLIENT_MAX_BODY_SIZE:
		std::cout << "\tclient_max_body_size: [" << value << "]" << std::endl;
		break;

	case AUTOINDEX:
		std::cout << "\tautoindex: [" << value << "]" << std::endl;
		if (value != "on" || value != "off")
			throw std::runtime_error("Invalid value for autoindex directive");
		if (value == "on")
			locationBlock.autoindex = true;
		if (value == "off")
			locationBlock.autoindex = false;
		std::cout << "\tautoindex: [" << locationBlock.autoindex << "]" << std::endl;
		break;

	case INDEX:
		std::cout << "\tindex: [" << value << "]" << std::endl;
		break;

	case RETURN:
		std::cout << "\treturn: [" << value << "]" << std::endl;
		break;

	case PATH_INFO:
		std::cout << "\tpath_info: [" << value << "]" << std::endl;
		break;

	case CGI:
		std::cout << "\tcgi_param: [" << value << "]" << std::endl;
		break;

	case UPLOAD_LOCATION:
		// std::cout << "\tupload_location: [" << value << "]" << std::endl;
		locationBlock.uploadLocation = value;
		break;

	case SET_METHOD:
		std::cout << "\tmethod: [" << value << "]" << std::endl;
		break;

	default:
		throw std::runtime_error("[setLocationValues]Unknown directive '" + expression + "'");
		break;
	}
}

void	Configuration::parseLocationBlock(std::stringstream &content, ServerBlock &serverBlock)
{
	LocationBlock	locationBlock;
	std::string		line;
	std::string		directive;
	std::string		value;

	initLocationBlock(locationBlock);
	while (std::getline(content, line))
	{
		if (line.empty())
			continue;
		if (line == "\t}")
			break;
		else
		{
			directive = extractDirective(line);
			value = extractValue(line);
			value.erase(0, directive.size() + 1);
			setLocationValues(directive, value, locationBlock);
		}
	}
	serverBlock.locationBlocks.push_back(locationBlock);
}

void	Configuration::parseServerBlock(std::stringstream &content)
{
	ServerBlock	server;
	std::string	line;
	std::string	directive;
	std::string	value;

	std::cout << std::endl << "server block" << std::endl;
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
		{
			directive = extractDirective(line);
			value = extractValue(line);
			value.erase(0, directive.size() + 1);
			// std::cout << "directive extracted: [" << directive << "]" << std::endl;
			// std::cout << "value extracted: [" << value << "]" << std::endl;
			setServerValues(directive, value, server);
		}
	}
	m_serverBlocks.push_back(server);
}

void	Configuration::parseConfigFile()
{
	std::string			line;

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

	initDirectiveMap();
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
			std::cout << "--LOCATION--" << std::endl;
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
