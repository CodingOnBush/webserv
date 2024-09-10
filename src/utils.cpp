#include "Webserv.hpp"
#include "Response.hpp"

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