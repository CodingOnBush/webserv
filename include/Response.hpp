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
#include "Cgi.hpp"

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
	LocationBlock getLocationFromServer(Configuration &config, Request &req);
	void handleRoot(std::string configPath, std::string requestUri);
	void bodySizeCheck(Configuration &config, LocationBlock &location);
	void methodCheck(LocationBlock location);
	void setErrorBody(LocationBlock location);
	std::string getBodyFromFile(std::string path);
	void getBody(std::string uri, LocationBlock location);
	std::string handleRedirection(Configuration &config, LocationBlock &location);
public:
	Response();
	Response(Request &req);
	~Response();
	std::string getResponse(Configuration &config);
	void setStatusLine();
	void createResponseStr(LocationBlock location);
	void handleGetRequest(Configuration &config, LocationBlock location);
	void handlePostRequest(Configuration &config, LocationBlock location);
	void handleDeleteRequest(Configuration &config, LocationBlock location);
	void setHeaders(LocationBlock location);
	void setMimeType(std::string const &fileName);
	void setStatusCode(int code);
	void setBody(std::string const &body);
};

std::string intToString(int value);
bool isDirectory( const std::string & path );
bool isFile( const std::string & path );
int serverBlocksCount(Configuration &config, std::string host, int port);
ServerBlock getDefaultServerBlock(Configuration &config, std::string host, int port);
bool matchExists(Configuration &config, std::string host, int port);
ServerBlock getMatchingServerBlock(Configuration &config, std::string host, int port);
bool locationBlockExists(ServerBlock serverBlock, std::string uri);
LocationBlock getMatchingLocationBlock(ServerBlock ServerBlock, std::string uri);
bool serverBlockExists(Configuration &config, Request &req);
std::string getDefaultErrorBody(int statusCode);
bool isInIndex(std::string fileName, LocationBlock location);
std::string getFilePath(std::string path, std::string uri, std::string fileName);
bool hasDefaultFile(const std::string& directoryPath, std::string fileName, LocationBlock location);
std::string setPath(LocationBlock location, std::string uri);
std::string generateDirectoryListingHTML(const std::string& directoryPath, const std::string &rootPath);