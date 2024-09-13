#include "Webserv.hpp"
#include "Response.hpp"

std::string intToString(int value)
{
	std::stringstream ss;
	ss << value;
	return ss.str();
}

bool isDirectory(const std::string &path)
{
	struct stat s;
	if (stat(path.c_str(), &s) == 0)
	{
		if (s.st_mode & S_IFDIR)
			return true;
	}
	return false;
}

bool isFile(const std::string &path)
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
bool isInIndex(std::string fileName, LocationBlock location)
{
	if (fileName == "index.html")
		return true;
	for (std::vector<std::string>::iterator it = location.indexes.begin(); it != location.indexes.end(); it++)
	{
		if (fileName == *it)
			return true;
	}
	return false;
}

bool hasDefaultFile(const std::string &directoryPath, std::string fileName, LocationBlock location)
{
	DIR *directoryPtr = opendir(directoryPath.c_str());
	struct dirent *dir;
	if (directoryPtr == NULL)
		return false;
	while ((dir = readdir(directoryPtr)) != NULL)
	{
		std::string fileName = dir->d_name;
		if (fileName == "." || fileName == "..")
			continue;
		if (fileName == "index.html")
		{
			closedir(directoryPtr);
			return true;
		}
		for (size_t i = 0; i < location.indexes.size(); ++i)
		{
			if (fileName == location.indexes[i])
			{
				closedir(directoryPtr);
				return true;
			}
		}
	}
	closedir(directoryPtr);
	return false;
}

std::string getFilePath(std::string path, std::string uri, std::string fileName)	
{
	std::string filePath;
	if (path[path.size() - 1] != '/')
		path += "/";
	if (uri != "/")
		filePath = path + fileName;
	else
		filePath = path + "index.html";
	return filePath;
}