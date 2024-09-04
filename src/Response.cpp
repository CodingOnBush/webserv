#include "Response.hpp"

Response::Response() {};

Response::Response(Request &req) : req(req) {};

Response::~Response() {};

std::string Response::getStatusMsg(int code)
{
	switch (code)
	{
	case 200:
		return "OK";
	case 201:
		return "Created";
	case 202:
		return "Accepted";
	case 204:
		return "No Content";
	case 400:
		return "Bad Request";
	case 401:
		return "Unauthorized";
	case 403:
		return "Forbidden";
	case 404:
		return "Not Found";
	case 405:
		return "Method Not Allowed";
	case 409:
		return "Conflict";
	case 411:
		return "Length Required";
	case 413:
		return "Payload Too Large";
	case 415:
		return "Unsupported Media Type";
	case 500:
		return "Internal Server Error";
	case 501:
		return "Not Implemented";
	case 505:
		return "HTTP Version Not Supported";
	}
	return "Internal Server Error";
}
void Response::setBody(std::string const &body)
{
	this->body = body;
}
void Response::setStatusLine()
{
	std::stringstream ss;
	ss << "HTTP/" << req.getVersion() << " " << statusCode << " " << getStatusMsg(statusCode) << "\n";
	statusLine = ss.str();
}

void Response::createResponseStr()
{
	std::stringstream ss;
	std::string statusLine = "HTTP/1.1 200 OK\n";
	std::string headers = "Content-Type: text/html\r\nContent-Length: 153\n\n";
	std::string body = "<!DOCTYPE html><html lang=\"en\"><head>  <title>A simple webpage</title></head><body>  <h1>Simple HTML webpage</h1>  <p>Hello, world!</p></body></html>\r\n\r\n";
	ss << statusLine << headers << body << "\n";
	response = ss.str();
}
void Response::handleGetRequest()
{
	// std::string headers = setHeaders(req, config, resp);
	// setBody("<!DOCTYPE html><html lang=\"en\"><head>  <title>A simple webpage</title></head>"
			// "<body>  <h1>Simple HTML webpage</h1>  <p>Hello, world!</p></body></html>\r\n\r\n");
	setStatusLine();
	createResponseStr();
}

void Response::handlePostRequest()
{
	return;
}

void Response::handleDeleteRequest()
{
	return;
}
std::string Response::getResponse()
{
	switch (req.getMethod())
	{
	case GET:
		handleGetRequest();
		break;
	case POST:
		handlePostRequest();
		break;
	case DELETE:
		handleDeleteRequest();
	// case UNKNOWN:
	// 	handleUnknownRequest();
		break;
	}
	return response;
}