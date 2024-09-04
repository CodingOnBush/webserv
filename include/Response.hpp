#pragma once
#include <string>
#include <map>
#include "Configuration.hpp"
#include "Request.hpp"

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

public:
	Response();
	Response(Request &req);
	~Response();
	std::string getResponse();
	void setStatusCode(int code);
	void setStatusLine();
	void createResponseStr();
	void handleGetRequest();
	void handlePostRequest();
	void handleDeleteRequest();
	// std::string getStatusCode() const;
	// std::string getStatusMsg() const;
	// void setHeader(const std::string &name, const std::string &value);
	// std::map<std::string, std::string> getHeaders() const;
	// std::string getBody() const;
};



