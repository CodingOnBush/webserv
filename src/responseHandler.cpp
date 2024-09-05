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

void handleDir(std::string path)
{
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
		struct dirent *dir;
		while ((dir = readdir(directoryPtr)) != NULL)
		{
			std::string fileName = dir->d_name;
			std::string filePath = path + "/" + fileName;
			if (fileName != "index.html")
				continue;
			std::ifstream file(filePath.c_str());
			std::stringstream body;
			if (file.is_open())
			{
				std::string line;
				while (std::getline(file, line))
				{
					body << line << std::endl;
				}
				file.close();
				// std::cout << body.str() << std::endl;
			}
			else
			{
				std::cout << "Failed to open file: " << filePath << std::endl;
			}
		}
		closedir(directoryPtr);
	}
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
		if (it->host == hostName && it->port == port)
		{
			std::cout << "Host found" << std::endl;
			std::string path = getPath(it, req.getUri());
			if (path.size() == 0)
			{
				// handle in separate function
				std::cout << "Path not found" << std::endl;
				return;
			}
			handleDir(path);
		}
	}
	std::cout << "Host not found" << std::endl;
}