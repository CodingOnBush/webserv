#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <cctype>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <utility>
#include <stdexcept>
#include "Configuration.hpp"

enum requestState
{
	RECEIVED,
	SENT,
};

class Request
{
private:
	std::string buffer;
	int method;
	std::string uri;
	std::string version;
	std::map<std::string, std::string> headers;
	std::map<std::string, std::string> queries;
	std::string body;
	int state;
	// parsing
	void parseRequest();
	void parseRequestLine(const std::string &line);
	void setUri(const std::string &str);
	void setMethod(const std::string &str);
	void setVersion(const std::string &str);
	void setHeaders(std::istringstream &stream);
	bool isValidHeader(const std::string &line, std::string &name, std::string &value);
	void parseHeaderName(const std::string &str, std::string &name);
	void parseHeaderValue(const std::string &str, std::string &value);
	bool parseBody(std::string &body);

public:
	// bool isDir;
	Request(const std::string &buffer);
	Request();
	~Request();
	// getters
	std::string getBuffer() const;
	int getMethod() const;
	std::string getUri() const;
	std::string getVersion() const;
	std::map<std::string, std::string> getHeaders() const;
	std::string getBody() const;
	void setRequestState(int state);
	int getRequestState();
	void clearRequest();
	// debug
	void printRequest(Request &req);
};