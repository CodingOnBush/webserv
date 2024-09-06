#pragma once
#include <string>
#include <map>
#include "Configuration.hpp"
#include "Request.hpp"
#include <dirent.h>
#include <unistd.h> 

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
	std::string getStatusMsg(int code);
	void setBody(std::string const &body);
	void processServerBlock(Configuration &config, Request &req);
	std::string getPath(std::vector<ServerBlock>::iterator it, std::string uri);
	void handleDir(std::string configPath, std::string requestUri);
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
};
