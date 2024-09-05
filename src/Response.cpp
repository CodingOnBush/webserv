#include "Response.hpp"

Response::Response() {};

Response::Response(Request &req) : req(req) {};

Response::~Response() {};

std::string Response::getStatusMsg(int code)
{
	switch (code)
	{
	case 200:
		return "OK";
	case 201:
		return "Created";
	case 202:
		return "Accepted";
	case 204:
		return "No Content";
	case 400:
		return "Bad Request";
	case 401:
		return "Unauthorized";
	case 403:
		return "Forbidden";
	case 404:
		return "Not Found";
	case 405:
		return "Method Not Allowed";
	case 409:
		return "Conflict";
	case 411:
		return "Length Required";
	case 413:
		return "Payload Too Large";
	case 415:
		return "Unsupported Media Type";
	case 500:
		return "Internal Server Error";
	case 501:
		return "Not Implemented";
	case 505:
		return "HTTP Version Not Supported";
	}
	return "Internal Server Error";
}
void Response::setBody(std::string const &body)
{
	this->body = body;
}
void Response::setStatusLine()
{
	std::stringstream ss;
	ss << "HTTP/" << req.getVersion() << " " << statusCode << " " << getStatusMsg(statusCode) << "\n";
	statusLine = ss.str();
}

void Response::createResponseStr()
{
	std::stringstream ss;
	std::string statusLine = "HTTP/1.1 200 OK\n";
	std::string headers = "Content-Type: text/html\r\nContent-Length: 153\n\n";
	ss << statusLine << headers << body << "\n";
	response = ss.str();
}

std::string Response::setHeaders(Request &req, Configuration &config, Response &resp)
{	
	// std::string headers = "Content-Type: text/html\r\nContent-Length: 153\n\n";
	// return headers;
	return "";	
}

void Response::handleGetRequest(Configuration &config)
{
	processServerBlock(config, this->req);
	
	createResponseStr();
}

void Response::handlePostRequest(Configuration &config)
{
	return;
}

void Response::handleDeleteRequest(Configuration &config)
{
	return;
}
std::string Response::getResponse(Configuration &config)
{
	switch (req.getMethod())
	{
	case GET:
		handleGetRequest(config);
		break;
	case POST:
		handlePostRequest(config);
		break;
	case DELETE:
		handleDeleteRequest(config);
	// case UNKNOWN:
	// 	handleUnknownRequest();
		break;
	}
	return response;
}

std::string Response::getPath(std::vector<ServerBlock>::iterator it, std::string uri)
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

void Response::handleDir(std::string path)
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
				this->body = body.str();
			}
			else
			{
				std::cout << "Failed to open file: " << filePath << std::endl;
			}
		}
		closedir(directoryPtr);
	}
}
void Response::processServerBlock(Configuration &config, Request &req)
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
			std::string path = getPath(it, req.getUri());
			if (path.size() == 0)
			{
				statusCode = 404;
				return;
			}
			handleDir(path);
		}
	}
}