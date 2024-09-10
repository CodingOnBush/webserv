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
	ss << req.getVersion() << " " << statusCode << " " << getStatusMsg(statusCode) << LF;
	statusLine = ss.str();
}

void Response::createResponseStr()
{
	std::stringstream ss;
	setStatusLine();
	setHeaders();
	ss << statusLine << headers << body << LF;
	response = ss.str();
}
void Response::setMimeType(std::string const &fileName)
{
	std::string extension = fileName.substr(fileName.find_last_of(".") + 1);
	if (extension == "html")
		mimeType = "text/html";
	else if (extension == "css")
		mimeType = "text/css";
	else if (extension == "js")
		mimeType = "text/javascript";
	else if (extension == "jpg")
		mimeType = "image/jpeg";
	else if (extension == "jpeg")
		mimeType = "image/jpeg";
	else if (extension == "png")
		mimeType = "image/png";
	else if (extension == "gif")
		mimeType = "image/gif";
	else if (extension == "bmp")
		mimeType = "image/bmp";
	else if (extension == "ico")
		mimeType = "image/x-icon";
	else if (extension == "svg")
		mimeType = "image/svg+xml";
	else if (extension == "mp3")
		mimeType = "audio/mpeg";
	else if (extension == "mp4")
		mimeType = "video/mp4";
	else if (extension == "webm")
		mimeType = "video/webm";
	else if (extension == "ogg")
		mimeType = "audio/ogg";
	else if (extension == "wav")
		mimeType = "audio/wav";
	else if (extension == "avi")
		mimeType = "video/x-msvideo";
	else if (extension == "mpeg")
		mimeType = "video/mpeg";
	else if (extension == "txt")
		mimeType = "text/plain";
	else
		mimeType = "application/octet-stream";
}
void Response::setHeaders()
{
	std::stringstream ss;
	ss << "Content-Type: " << mimeType << CRLF
	   << "Content-Length: " << body.size() << CRLF << CRLF;
	headers = ss.str();
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
	for (std::vector<LocationBlock>::iterator it = locationBlocks.begin(); it != locationBlocks.end(); it++)
	{
		std::cout << "PATH: " << it->path << std::endl;
		std::cout << "URI: " << it->root << std::endl;
		if (it->path == uri)
		{
			return it->root;
		}
	}
	return "";
}

void Response::handleRoot(std::string configPath, std::string requestUri)
{
	DIR *directoryPtr = opendir(configPath.c_str());
	if (directoryPtr == NULL)
	{
		if (errno == ENOENT)
		{
			std::cout << "Directory does not exist" << std::endl;
			this->statusCode = 404;
			return;
		}
		else if (errno == EACCES)
		{
			this->statusCode = 403;
			return;
		}
		else
		{
			this->statusCode = 500;
			return;
		}
	}
	else
	{
		struct dirent *dir;
		while ((dir = readdir(directoryPtr)) != NULL)
		{
			std::string fileName = dir->d_name;
			if (fileName == "." || fileName == "..")
				continue;
			std::string filePath = configPath + "/" + fileName;
			if (requestUri == "/")
			{
				filePath = configPath + "/" + "index.html";
			}
			if (requestUri == "/" || requestUri == "/" + fileName)
			{
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
					if (requestUri == "/")
					{
						this->setMimeType("index.html");
					}
					else
					{
						this->setMimeType(fileName);
					}
					
					this->statusCode = 200;
					break;
				}
				else
				{
					body << http_error_403_page << std::endl;
					this->body = body.str();
					this->statusCode = 403;
					break;
				}
			}
		}
		if (this->statusCode == 0)
		{
			std::cout << "404 - not found" << std::endl;
			this->statusCode = 404;
		}
		closedir(directoryPtr);
	}
};

void Response::processServerBlock(Configuration &config, Request &req)
{
	std::vector<ServerBlock> serverBlocks = config.getServerBlocks();
	ServerBlock serverBlock;
	LocationBlock location;
	std::string hostName = req.getHost();
	int port = req.getPort();
	int serverBlockCount = serverBlocksCount(config, hostName, port);
	if (serverBlockCount == 0)
		return;
	else if (serverBlockCount > 1 && matchExists(config, hostName, port))
		serverBlock = getMatchingServerBlock(config, hostName, port);
	else
	{
		serverBlock = getDefaultServerBlock(config, hostName, port);
	}
	std::string uri = req.getUri();
	if (locationBlockExists(serverBlock, uri))
	{
		location = getMatchingLocationBlock(serverBlock, uri);
		if (location.path.size() == 0)
		{
			statusCode = 404;
			return;
		}
		handleRoot(location.root, uri);
	}
	else
	{
		location = getMatchingLocationBlock(serverBlock, "/");
		// std::cout << "Location Path2: " << location.path << std::endl;
		handleRoot(serverBlock.root, uri);
	}
	
}