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

#define CRLF "\r\n" // Carriage Return Line Feed
#define CRLF2 "\r\n\r\n"

enum requestState
{
	RECEIVED,
	SENT,
};

enum parsingState
{
	REQUEST_LINE,
	HEADERS,
	BODY,
	PARSING_DONE
};

class Request
{
private:
	int method;
	std::string uri;
	std::string version;
	std::map<std::string, std::string> headers;
	// std::map<std::string, std::string> queries;
	std::string body;
	int parsingState;
	int state;
	// parsing
	void parseRequestLine(const std::string &line);
	void setParsingState(int state);
	void setUri(const std::string &str);
	void setMethod(const std::string &str);
	void setVersion(const std::string &str);
	void setHeaders(std::stringstream &stream);
	bool isValidHeader(const std::string &line, std::string &name, std::string &value);
	void parseHeaderName(const std::string &str, std::string &name);
	void parseHeaderValue(const std::string &str, std::string &value);
	void parseBody(std::stringstream &stream);
	bool hasBody();

public:
	// bool isDir;
	Request();
	~Request();
	// getters
	int getMethod() const;
	std::string getUri() const;
	std::string getVersion() const;
	std::map<std::string, std::string> getHeaders() const;
	std::string getBody() const;
	void setRequestState(int state);
	int getRequestState();
	void clearRequest();
	void parseRequest(std::stringstream &stream);
	// debug
	void printRequest(Request &req);
};