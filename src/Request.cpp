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

void Request::setUri(const std::string &str)
{
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (std::isspace(str[i]))
            throw std::logic_error("Invalid URI: " + str);
        this->uri += str[i];
    }
}

void Request::setMethod(const std::string &str)
{
    if (str == "GET" || str == "DELETE" || str == "POST")
    {
        this->method = str;
    }
    else
    {
        throw std::logic_error("Invalid method: " + str);
    }
}

void Request::setVersion(const std::string &str)
{
    if (str.find("HTTP/") == 0)
    {
        this->version = str;
        this->version.erase(0, 5);
    }
    else
    {
        throw std::logic_error("Invalid version: " + str);
    }
}

void Request::setHeaders(std::istringstream &stream)
{
    std::string line;

    while (std::getline(stream, line))
    {
        std::string name, value;
        if (!isValidHeader(line, name, value) || line.empty())
            break;
        headers[name] = value;
    }
}

bool Request::isValidHeader(const std::string &line, std::string &name, std::string &value)
{
    size_t pos = line.find(':');
    if (pos == std::string::npos)
        return false;

    std::string header_name = line.substr(0, pos);
    std::string header_value = line.substr(pos + 1);

    try
    {
        parseHeaderName(header_name, name);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
    try
    {
        parseHeaderValue(header_value, value);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
    return true;
}

void Request::parseHeaderName(const std::string &str, std::string &name)
{
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (str[i] == '\n' || str[i] == ':')
            throw std::logic_error("Invalid header: " + str);
        name += str[i];
    }
}

void Request::parseHeaderValue(const std::string &str, std::string &value)
{
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (str[i] == '\n')
            throw std::logic_error("Invalid header value: " + str);
        value += str[i];
    }
}

bool Request::parseBody(std::string &body)
{
    (void)body;
    // unchunking body logic
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
void Request::parseRequestLine(const std::string &line)
{
    std::istringstream stream(line);
    std::string method, uri, version;

    stream >> method;
    try
    {
        setMethod(method);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

    stream >> uri;
    try
    {
        setUri(uri);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

    stream >> version;
    try
    {
        setVersion(version);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
}

void Request::parseRequest()
{
    std::string line;
    std::istringstream stream(buffer);

    // check if buffer is empty and protect
    std::getline(stream, line);
    parseRequestLine(line);
    setHeaders(stream);
    body = buffer.substr(buffer.find("\r\n\r\n") + 4, buffer.length());
    // parseBody(body);
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
