#pragma once
#include <string>
#include <map>
#include "Configuration.hpp"
#include "Request.hpp"
#include <dirent.h>
#include <unistd.h> 
#include <sys/stat.h>
#include <string.h>
#include "DefaultErrors.hpp"

class Configuration;
class Request;

class Response
{
private:
	Request req;
	std::string statusLine;
	int statusCode;
	std::string headers;
	std::string body;
	std::string response;
	std::string mimeType;
	std::string getStatusMsg(int code);
	void setBody(std::string const &body);
	void processServerBlock(Configuration &config, Request &req);
	std::string getPath(std::vector<ServerBlock>::iterator it, std::string uri);
	void handleRoot(std::string configPath, std::string requestUri);
	
public:
	Response();
	Response(Request &req);
	~Response();
	std::string getResponse(Configuration &config);
	void setStatusLine();
	void createResponseStr();
	void handleGetRequest(Configuration &config);
	void handlePostRequest(Configuration &config);
	void handleDeleteRequest(Configuration &config);
	void setHeaders();
	void setMimeType(std::string const &fileName);
};


bool isDirectory( const std::string & path );
bool isFile( const std::string & path );
int serverBlocksCount(Configuration &config, std::string host, int port);
ServerBlock getDefaultServerBlock(Configuration &config, std::string host, int port);
bool matchExists(Configuration &config, std::string host, int port);
ServerBlock getMatchingServerBlock(Configuration &config, std::string host, int port);
bool locationBlockExists(ServerBlock serverBlock, std::string uri);
LocationBlock getMatchingLocationBlock(ServerBlock ServerBlock, std::string uri);