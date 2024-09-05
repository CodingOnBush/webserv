#include "Response.hpp"
#include "Configuration.hpp"
#include "Webserv.hpp"

std::string getPath(std::vector<ServerBlock>::iterator it, std::string uri)
{
	std::vector<LocationBlock> locationBlocks = it->locationBlocks;

	std::cout << "URI: " << uri << std::endl;
	for (std::vector<LocationBlock>::iterator it = locationBlocks.begin(); it != locationBlocks.end(); it++)
	{
		std::cout << "PATH: " << it->path << std::endl;

		if (it->path == uri)
		{
			return it->root;
		}
	}
	return "";
}
void processServerBlock(Configuration &config, Request &req)
{
	std::vector<ServerBlock>::iterator it;
	std::vector<ServerBlock> serverBlocks = config.getServerBlocks();
	std::string host = req.getHeaders()["Host"];
	size_t pos = host.find(':');
	std::string hostName = host.substr(0, pos);
	std::string portValue = host.substr(pos + 1);
	int port;
	std::stringstream ss(portValue);
	ss >> port;
	for (std::vector<ServerBlock>::iterator it = serverBlocks.begin(); it != serverBlocks.end(); it++)
	{
		std::cout << "Request Host: " << hostName << std::endl;
		std::cout << "Request Port: " << port << std::endl;
		std::cout << "Config Host: " << it->host << std::endl;
		std::cout << "Config Port: " << it->port << std::endl;
		if (it->host == hostName && it->port == port)
		{
			std::cout << "Host found" << std::endl;
			std::string path = getPath(it, req.getUri());
			std::cout << "Root: " << path << std::endl;
			DIR *directoryPtr = opendir(path.c_str());
			if (directoryPtr == NULL)
			{
				if (errno == ENOENT)
				{
					std::cout << "Directory not found" << std::endl;
					return;
				}
				else if (errno == EACCES)
				{
					std::cout << "Directory access denied" << std::endl;
					return;
				}
				else
				{
					std::cout << "Directory error" << std::endl;
					return;
				}
			}
			else
			{
				std::cout << "Directory found" << std::endl;
				return;
			}
		}
	}
	std::cout << "Host not found" << std::endl;
}