#include "Request.hpp"

Request::Request(std::string buffer) : buffer(buffer) {};

Request::~Request() {};

std::string Request::getBuffer() const
{
	return buffer;
}

std::string Request::getMethod() const
{
	return method;
}

std::string Request::getUri() const
{
	return uri;
}

std::string Request::getVersion() const
{
	return version;
}

std::map<std::string, std::string> Request::getHeaders() const
{
	return headers;
}

std::vector<char> Request::getBody() const
{
	return body;
}

bool Request::parseRequestLine(const std::string &line) {
    std::istringstream stream(line);
    std::string method, uri, version;

    stream >> method;
    if (!parseMethod(method, this->method)) return false;

    stream >> uri;
    if (!parseUri(uri, this->uri)) return false;

    stream >> version;
    if (!parseVersion(version, this->version)) return false;

    return true;
}

bool Request::parseUri(const std::string &str, std::string &uri) {
    for (size_t i = 0; i < str.size(); ++i) {
        if (parseWhitespace(str[i])) return false;
        uri += str[i];
    }
    return !uri.empty();
}

bool Request::parseMethod(const std::string &str, std::string &method) {
    if (str == "GET" || str == "DELETE" || str == "POST" || str == "PUT") {
        method = str;
        return true;
    }
    return false;
}

bool Request::parseVersion(const std::string &str, std::string &version) {
    if (str.find("HTTP/") == 0) {
		//remove "HTTP/"
        version = str;
        return true;
    }
    return false;
}

bool Request::parseWhitespace(char c) {
    return c == ' ' || c == '\t';
}

//segfault somewhere here
bool Request::parseHeaders(const std::vector<std::string> &lines) {
    for (size_t i = 0; i < lines.size(); ++i) {
        std::string name, value;
        if (!parseHeader(lines[i], name, value)) return false;
        headers[name] = value;
    }
    return true;
}

bool Request::parseHeader(const std::string &line, std::string &name, std::string &value) {
    size_t pos = line.find(':');
    if (pos == std::string::npos) return false;

    std::string header_name = line.substr(0, pos);
    std::string header_value = line.substr(pos + 1);

    if (!parseHeaderName(header_name, name)) return false;
    if (!parseHeaderValue(header_value, value)) return false;

    return true;
}

bool Request::parseHeaderName(const std::string &str, std::string &name) {
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '\n' || str[i] == ':') return false;
        name += str[i];
    }
    return !name.empty();
}

bool Request::parseHeaderValue(const std::string &str, std::string &value) {
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '\n') return false;
        value += str[i];
    }
    return !value.empty();
}

bool Request::parseBody(const std::string &str, std::vector<char> &body) {
    for (size_t i = 0; i < str.size(); ++i) {
        if (parseDelimiter(str.substr(i))) return false;
        body.push_back(str[i]);
    }
    return true;
}
bool Request::parseDelimiter(const std::string &str) {
    return str.substr(0, 3) == "###" && str.find('\n') != std::string::npos;
}

bool Request::parseRequest()
{
	std::vector<std::string> lines;
	std::string line;
	std::istringstream stream(buffer);

	while (std::getline(stream, line))
	{
		if (line.empty())
			break;
		lines.push_back(line);
	}

	if (lines.empty())
		return false;

	if (!parseRequestLine(lines[0]))
		return false;

	if (!parseHeaders(std::vector<std::string>(lines.begin() + 1, lines.end())))
		return false;

	return true;
}

void Request::printRequest(Request &req)
{
	std::cout << "Request buffer:" << std::endl;
	std::cout << req.getBuffer() << std::endl;
	std::cout << std::string(21, '*') << std::endl;
	std::cout << "Method: " << req.getMethod() << std::endl;
	std::cout << "URI: " << req.getUri() << std::endl;
	std::cout << "Version: " << req.getVersion() << std::endl;
	std::cout << "Headers: " << std::endl;
	for (std::map<std::string, std::string>::iterator it = req.getHeaders().begin(); it != req.getHeaders().end(); ++it)
	{
		std::cout << it->first << ":" << it->second << std::endl;
	}
	//overload operator<< for std::vector<char> required
	// std::cout << "Body: " << req.getBody() << std::endl;
}
