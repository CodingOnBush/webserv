#pragma once
#include <string>
#include <map>
#include "Configuration.hpp"
#include "Request.hpp"

std::map<int, std::string> statusMessages = {
	{200, "OK"},
	{201, "Created"},
	{202, "Accepted"},
	{204, "No Content"},
	{400, "Bad Request"},
	{401, "Unauthorized"},
	{403, "Forbidden"},
	{404, "Not Found"},
	{405, "Method Not Allowed"},
	{409, "Conflict"},
	{411, "Length Required"},
	{413, "Payload Too Large"},
	{415, "Unsupported Media Type"},
	{500, "Internal Server Error"},
	{501, "Not Implemented"},
	{505, "HTTP Version Not Supported"}};
	
class Configuration;
class Request;

class Response
{
private:
	std::string statusLine; // HTTP/1.1 404 Not Found OR HTTP/1.1 200 OK
	std::map<std::string, std::string> headers;
	std::string body;
	// void setStatusCode();
	// void setStatusMsg();
	// void setHeader(const std::string &name, const std::string &value);
	// void setBody(const std::string &body);

public:
	Response();
	~Response();

	// std::string getStatusCode() const;
	// std::string getStatusMsg() const;
	// std::map<std::string, std::string> getHeaders() const;
	// std::string getBody() const;
	std::string getResponse(Request &req, Configuration &config) const;
	std::string setStatusLine(Request &req);
};
