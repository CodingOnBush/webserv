#include "Webserv.hpp"
#include "Response.hpp"

std::string intToString(int value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}

bool	isDirectory( const std::string & path )
{
	struct stat s;
	if (stat(path.c_str(), &s) == 0)
	{
		if (s.st_mode & S_IFDIR)
			return true;
	}
	return false;
}

bool	isFile( const std::string & path )
{
	struct stat s;
	if (stat(path.c_str(), &s) == 0)
	{
		if (s.st_mode & S_IFREG)
			return true;
	}
	return false;
}
bool serverBlockExists(Configuration &config, Request &req)
{
	std::vector<ServerBlock> serverBlocks = config.getServerBlocks();
	std::string host = req.getHost();
	int port = req.getPort();
	for (std::vector<ServerBlock>::iterator it = serverBlocks.begin(); it != serverBlocks.end(); it++)
	{
		if (it->host == host && it->port == port)
			return true;
	}
	return false;
}
int serverBlocksCount(Configuration &config, std::string host, int port)
{
	size_t count = 0;
	std::vector<ServerBlock> serverBlocks = config.getServerBlocks();
	for (std::vector<ServerBlock>::iterator it = serverBlocks.begin(); it != serverBlocks.end(); it++)
	{
		if (it->host == host && it->port == port)
			count++;
	}
	return count;
}

ServerBlock getDefaultServerBlock(Configuration &config, std::string host, int port)
{
	ServerBlock *serverBlock = NULL;
	std::vector<ServerBlock> serverBlocks = config.getServerBlocks();
	for (std::vector<ServerBlock>::iterator it = serverBlocks.begin(); it != serverBlocks.end(); it++)
	{
		if (it->host == host && it->port == port)
			return *it;
	}
	return *serverBlock;
}

bool matchExists(Configuration &config, std::string host, int port)
{
    std::vector<ServerBlock> serverBlocks = config.getServerBlocks();
    for (std::vector<ServerBlock>::iterator it = serverBlocks.begin(); it != serverBlocks.end(); ++it)
    {
        for (std::vector<std::string>::iterator it2 = it->serverNames.begin(); it2 != it->serverNames.end(); ++it2)
        {
            if (it->host == host && it->port == port && *it2 == host)
                return true;
        }
    }
    return false;
}

ServerBlock getMatchingServerBlock(Configuration &config, std::string host, int port)
{
	ServerBlock *serverBlock = NULL;
	std::vector<ServerBlock> serverBlocks = config.getServerBlocks();
	for (std::vector<ServerBlock>::iterator it = serverBlocks.begin(); it != serverBlocks.end(); it++)
	{
		for (std::vector<std::string>::iterator it2 = it->serverNames.begin(); it2 != it->serverNames.end(); ++it2)
        {
            if (it->host == host && it->port == port && *it2 == host)
                return *it;
        }
	}
	return *serverBlock;
}

bool locationBlockExists(ServerBlock serverBlock, std::string uri)
{
	std::vector<LocationBlock> locationBlocks = serverBlock.locationBlocks;
	for (std::vector<LocationBlock>::iterator it = locationBlocks.begin(); it != locationBlocks.end(); it++)
	{
		if (it->path == uri)
		{
			return true;
		}
	}
	return false;
}

LocationBlock getMatchingLocationBlock(ServerBlock serverBlock, std::string uri)
{
	LocationBlock *location = NULL;
	std::vector<LocationBlock> locationBlocks = serverBlock.locationBlocks;
	for (std::vector<LocationBlock>::iterator it = locationBlocks.begin(); it != locationBlocks.end(); it++)
	{
		if (it->path == uri)
		{
			return *it;
		}
	}
	return *location;
}

std::string getDefaultErrorBody(int statusCode)
{
	for (std::map<int, std::string>::const_iterator it = http_error_pages.begin(); it != http_error_pages.end(); it++)
	{
		if (it->first == statusCode)
			return it->second;
	}
	return http_error_500_page;
}
// void getBody(std::string rootPath, std::string uri) {
//     std::cout << "rootPath: " << rootPath << std::endl;
//     std::cout << "uri: " << uri << std::endl;
//     std::string path = rootPath + uri;
//     std::cout << "path: " << path << std::endl;

//     if (isDirectory(path)) {
//         std::cout << "This is a directory" << std::endl;
//         // Handle directory-specific behavior here
//     } else if (isFile(path)) {
//         std::cout << "This is a file" << std::endl;
//         // Handle file-specific behavior here
//     } else {
//         std::cout << "This is not a file or directory" << std::endl;
//         // Handle error case here
//     }
// }

bool hasDefaultFile(std::string fileName, LocationBlock location)
{
	std::cout << "CHECK fileName: " << fileName << std::endl;
	if (fileName == "index.html")
		return true;
	for(std::vector<std::string>::iterator it = location.indexes.begin(); it != location.indexes.end(); it++)
	{
		if (fileName == *it)
			return true;
	}
	return false;
}