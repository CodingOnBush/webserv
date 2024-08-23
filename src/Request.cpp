#include "Request.hpp"

Request::Request(const std::string &buffer) : buffer(buffer)
{
    parseRequest();
};

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

std::string Request::getBody() const
{
    return body;
}

bool Request::parseUri(const std::string &str, std::string &uri)
{
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (parseWhitespace(str[i]))
            return false;
        uri += str[i];
    }
    return !uri.empty();
}

bool Request::parseMethod(const std::string &str, std::string &method)
{
    if (str == "GET" || str == "DELETE" || str == "POST")
    {
        method = str;
        return true;
    }
    return false;
}

bool Request::parseVersion(const std::string &str, std::string &version)
{
    if (str.find("HTTP/") == 0)
    {
        version = str;
        version.erase(0, 5);
        return true;
    }
    return false;
}

bool Request::parseWhitespace(char c)
{
    return c == ' ' || c == '\t';
}

bool Request::parseHeaders(std::istringstream &stream)
{
    std::string line;

    while (std::getline(stream, line))
    {
        std::string name, value;
        if (!parseHeader(line, name, value) || line.empty())
            break;
        headers[name] = value;
    }
    return true;
}

bool Request::parseHeader(const std::string &line, std::string &name, std::string &value)
{
    size_t pos = line.find(':');
    if (pos == std::string::npos)
        return false;

    std::string header_name = line.substr(0, pos);
    std::string header_value = line.substr(pos + 1);

    if (!parseHeaderName(header_name, name))
        return false;
    if (!parseHeaderValue(header_value, value))
        return false;
    return true;
}

bool Request::parseHeaderName(const std::string &str, std::string &name)
{
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (str[i] == '\n' || str[i] == ':')
            return false;
        name += str[i];
    }
    return !name.empty();
}

bool Request::parseHeaderValue(const std::string &str, std::string &value)
{
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (str[i] == '\n')
            return false;
        value += str[i];
    }
    return !value.empty();
}

bool Request::parseBody(std::string &body)
{
    (void)body;
    // if (body.empty())
    //     return true;
    // std::string newBody = "";
    // size_t clen = atoi((*this)["Content-Length"].c_str());
    // if (!((*this)["Transfer-Encoding"] == "chunked")) return;
    // while (newBody.length() < clen) {
    //   int position = body.find("\r\n");
    //   int len = std::strtol(body.substr(0, position).c_str(), NULL, 16);
    //   body = body.substr(position + 2, body.length());
    //   newBody.append(this->body.substr(0, len));
    //   body = body.substr(len + 1, body.length());
    // }
    // this->body = newBody;
    return true;
}
bool Request::parseRequestLine(const std::string &line)
{
    std::istringstream stream(line);
    std::string method, uri, version;

    stream >> method;
    if (!parseMethod(method, this->method))
        return false;

    stream >> uri;
    if (!parseUri(uri, this->uri))
        return false;

    stream >> version;
    if (!parseVersion(version, this->version))
        return false;

    return true;
}

bool Request::parseRequest()
{
    std::string line;
    std::istringstream stream(buffer);

    std::getline(stream, line);
    parseRequestLine(line);
    parseHeaders(stream);
    body = buffer.substr(buffer.find("\r\n\r\n") + 4, buffer.length());
    // parseBody(body);
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
    std::map<std::string, std::string> headers = req.getHeaders();
    for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
    {
        std::cout << it->first << ":" << it->second << std::endl;
    }
    std::cout << "Body: " << std::endl;
    std::cout << req.getBody() << std::endl;
    std::cout << std::endl;
    std::cout << std::string(21, '*') << std::endl;
}
