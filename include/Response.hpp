#pragma once
#include <string>
#include <map>
#include "Configuration.hpp"
#include "Request.hpp"
#include <dirent.h>

class Configuration;
class Request;

class Response
{
private:
	Request req;
	std::string statusLine; // HTTP/1.1 404 Not Found OR HTTP/1.1 200 OK
	int statusCode;
	std::map<std::string, std::string> headers;
	std::string body;
	std::string response;
	std::string getStatusMsg(int code);
	void setBody(std::string const &body);
	void processServerBlock(Configuration &config, Request &req);
	std::string getPath(std::vector<ServerBlock>::iterator it, std::string uri);
	void handleDir(std::string path);
public:
	Response();
	Response(Request &req);
	~Response();
	std::string getResponse(Configuration &config);
	void setStatusCode(int code);
	void setStatusLine();
	void createResponseStr();
	void handleGetRequest(Configuration &config);
	void handlePostRequest(Configuration &config);
	void handleDeleteRequest(Configuration &config);
	std::string setHeaders(Request &req, Configuration &config, Response &resp);
};
