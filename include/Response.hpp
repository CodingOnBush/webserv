#pragma once
#include <string>
#include <map>

class Response
{
private:
	std::string statusCode;
	std::string statusMsg;
	std::map<std::string, std::string> headers;
	std::string body;
	void setStatusCode();
	void setStatusMsg();
	void setHeader(const std::string &name, const std::string &value);
	void setBody(const std::string &body);

public:
	Response();
	~Response();

	std::string getStatusCode() const;
	std::string getStatusMsg() const;
	std::map<std::string, std::string> getHeaders() const;
	std::string getBody() const;
	std::string getResponse() const;
};