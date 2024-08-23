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

// Request       = Request-Line              ; Section 5.1
// *(( general-header        ; Section 4.5
//  | request-header         ; Section 5.3
//  | entity-header ) CRLF)  ; Section 7.1
// CRLF
// [ message-body ]          ; Section 4.3


// PEST Grammar (src: https://pest.rs/)
// file = { SOI ~ (delimiter | request)* ~ EOI}

// request = {	
// 	request_line ~
//     headers? ~
//     NEWLINE ~
//     body?
// }

// request_line = _{ method ~ " "+ ~ uri ~ " "+ ~ "HTTP/" ~ version ~ NEWLINE }
// uri = { (!whitespace ~ ANY)+ }
// method = { ("GET" | "DELETE" | "POST" | "PUT") }
// version = { (ASCII_DIGIT | ".")+ }
// whitespace = _{ " " | "\t" }

// headers = { header+ }
// header = { header_name ~ ":" ~ whitespace ~ header_value ~ NEWLINE }
// header_name = { (!(NEWLINE | ":") ~ ANY)+ }
// header_value = { (!NEWLINE ~ ANY)+ }

// body = { (!delimiter ~ ANY)+ }
// delimiter = { "#"{3} ~ NEWLINE+ }

class Request
{
private:
	std::string buffer;
	std::string method;
	std::string uri;
	std::string version;
	std::map<std::string, std::string> headers;
	std::map<std::string, std::string> queries;
	std::string body;

public:
	Request(const std::string &buffer);
	~Request();
	//getters
	std::string getBuffer() const;
	std::string getMethod() const;
	std::string getUri() const;
	std::string getVersion() const;
	std::map<std::string, std::string> getHeaders() const;
	std::string getBody() const;
	//parsing
	void parseRequest();
	void parseRequestLine(const std::string &line);
    void parseUri(const std::string &str, std::string &uri);
    void parseMethod(const std::string &str, std::string &method);
    void parseVersion(const std::string &str, std::string &version);
    bool parseWhitespace(char c);
   	bool parseHeaders(std::istringstream &stream);
    bool parseHeader(const std::string &line, std::string &name, std::string &value);
    bool parseHeaderName(const std::string &str, std::string &name);
    bool parseHeaderValue(const std::string &str, std::string &value);
	bool parseBody(std::string &body);
	//debug
	void printRequest(Request &req);
};