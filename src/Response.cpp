#include "Response.hpp"

Response::Response() {};

Response::Response(Request &req) : req(req), statusCode(0) {};

Response::~Response() {};

void Response::setStatusCode(int code)
{
	this->statusCode = code;
}

void Response::setBody(std::string const &body)
{
	this->body = body;
}

// change this to setErrorBody ?
void Response::setErrorBody(LocationBlock location)
{
	if (location.errorPages.empty())
	{
		body = getDefaultErrorBody(this->statusCode);
	}
	else
	{
		body = getBodyFromFile(location.errorPages[intToString(this->statusCode)]);
	}
}

std::string Response::getBodyFromFile(std::string filePath)
{
	std::ifstream file(filePath.c_str());
	std::stringstream body;
	std::string fileName = filePath.substr(filePath.find_last_of("/") + 1);
	if (file.is_open())
	{
		std::string line;
		while (std::getline(file, line))
		{
			body << line << std::endl;
		}
		file.close();
		this->setMimeType(fileName);
		return body.str();
	}
	else
	{
		this->statusCode = 500;
		return http_error_500_page;
	}
}

void Response::setStatusLine()
{
	std::stringstream ss;
	ss << req.getVersion() << " " << statusCode << " " << getStatusMsg(statusCode) << LF;
	statusLine = ss.str();
}

void Response::createResponseStr(LocationBlock location)
{
	std::stringstream ss;
	setStatusLine();
	if (this->statusCode >= 400)
		setErrorBody(location);
	setHeaders(location);
	ss << statusLine << headers << body << LF;
	response = ss.str();
}
void Response::setMimeType(std::string const &fileName)
{
	if (fileName == "html")
	{
		mimeType = "text/html";
		return;
	}
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
void Response::setHeaders(LocationBlock location)
{
	std::stringstream ss;
	// add text/html as a default mime type (for default errors) ?
	ss << "Content-Type: " << mimeType << CRLF
	   << "Content-Length: " << body.size() << CRLF;
	if (location.redirection)
	{
		ss << "Location: " << location.redirects[statusCode] << CRLF;
	}
	ss << CRLF;
	headers = ss.str();
}

void Response::getBody(std::string uri, LocationBlock location)
{
	std::string path = setPath(location, uri);
	// std::cout << "Path: " << path << std::endl;
	if (isDirectory(path))
	{
		DIR *directoryPtr = opendir(path.c_str());
		if (directoryPtr == NULL)
		{
			if (errno == ENOENT)
			{
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
				if (hasDefaultFile(path, fileName, location))
				{
					if (!isInIndex(fileName, location))
						continue;
					std::ifstream file(getFilePath(path, uri, fileName).c_str());
					if (file.is_open())
					{
						std::stringstream body;
						std::string line;
						while (std::getline(file, line))
							body << line << std::endl;
						this->body = body.str();
						this->setMimeType(fileName);
						this->statusCode = 200;
						file.close();
					}
					else
						this->statusCode = 403;
					break;
				}
				else if (location.autoindex) // directory listing
				{
					this->body = generateDirectoryListingHTML(path, location.root);
					this->setMimeType("html");
					this->statusCode = 200;
					break;
				}
				else
				{
					this->statusCode = 403;
					break;
				}
			}
		}
		if (this->statusCode == 0)
			this->statusCode = 404;
		closedir(directoryPtr);
	}
	else if (isFile(path))
	{
		std::ifstream file(path.c_str());
		if (file.is_open())
		{
			std::string line;
			std::stringstream body;
			while (std::getline(file, line))
				body << line << std::endl;
			this->body = body.str();
			this->setMimeType(path);
			this->statusCode = 200;
			file.close();
		}
		else
			this->statusCode = 403;
	}
	else
		this->statusCode = 404;
}

void Response::handleGetRequest(Configuration &config, LocationBlock location)
{
	if (location.cgiParams.empty())
	{
		getBody(req.getUri(), location);
		return;
	}
	handleCGI(config, location, req, *this);
	// handle cgi
}

void Response::handlePostRequest(Configuration &config, LocationBlock locaion)
{
	if (locaion.cgiParams.empty())
	{
		this->statusCode = 405;
		return;
	}
	// handle cgi
	handleCGI(config, locaion, req, *this);
	return;
}

void Response::handleDeleteRequest(Configuration &config, LocationBlock location)
{
	if (location.cgiParams.empty())
	{
		this->statusCode = 405;
		return;
	}
	// handle delete
	handleCGI(config, location, req, *this);	
	return;
}

void Response::methodCheck(LocationBlock location)
{
	if (req.getMethod() == UNKNOWN)
	{
		this->statusCode = 405;
		return;
	}
	if (location.methods.empty() || std::find(location.methods.begin(), location.methods.end(), req.getMethod()) == location.methods.end())
	{
		this->statusCode = 405;
	}
}

void Response::bodySizeCheck(Configuration &config, LocationBlock &location)
{
	int maxBodySize = config.getBodySize(location.clientMaxBodySize);
	if (maxBodySize == 0)
		return;
	// compare with content length header!!!
	if (req.getBody().size() > maxBodySize)
	{
		if (this->statusCode == 0)
			this->statusCode = 413;
	}
}

std::string Response::handleRedirection(Configuration &config, LocationBlock &location)
{
	std::stringstream ss;
	body = "";
	setMimeType("html");
	statusCode = 307;
	setStatusLine();
	setHeaders(location);
	createResponseStr(location);
	return response;
}

std::string Response::getResponse(Configuration &config)
{
	LocationBlock location;
	if (serverBlockExists(config, this->req))
	{
		location = getLocationFromServer(config, this->req);
		methodCheck(location);
		bodySizeCheck(config, location);
		if (location.redirection == true)
			return (handleRedirection(config, location));
		if (this->statusCode == 0)
		{
			switch (req.getMethod())
			{
			case GET:
				handleGetRequest(config, location);
				break;
			case POST:
				handlePostRequest(config, location);
				break;
			case DELETE:
				handleDeleteRequest(config, location);
				break;
			}
		}
	}
	createResponseStr(location);
	return response;
}

void Response::handleRoot(std::string configPath, std::string requestUri)
{
	DIR *directoryPtr = opendir(configPath.c_str());
	if (directoryPtr == NULL)
	{
		if (errno == ENOENT)
		{
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
			std::cout << "fileName: " << fileName << std::endl;
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
					this->statusCode = 403;
					break;
				}
			}
		}
		if (this->statusCode == 0)
		{
			this->statusCode = 404;
		}
		closedir(directoryPtr);
	}
};

LocationBlock Response::getLocationFromServer(Configuration &config, Request &req)
{
	ServerBlock serverBlock;
	LocationBlock location;
	std::string hostName = req.getHost();
	int port = req.getPort();
	int serverBlockCount = serverBlocksCount(config, hostName, port);
	if (serverBlockCount > 1 && matchExists(config, hostName, port))
		serverBlock = getMatchingServerBlock(config, hostName, port);
	else
		serverBlock = getDefaultServerBlock(config, hostName, port);
	std::string uri = req.getUri();
	if (locationBlockExists(serverBlock, uri))
		location = getMatchingLocationBlock(serverBlock, uri);
	else if (serverBlock.locationBlocks.size() == 0)
		location = serverBlock.locationBlocks[0];
	else
		location = getMatchingLocationBlock(serverBlock, "/");
	return location;
}