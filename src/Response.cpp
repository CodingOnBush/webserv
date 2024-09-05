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
	std::string headers = "Content-Type: text/html\r\nContent-Length: 0\n\n";
	// std::string body = "<!DOCTYPE html><html lang=\"en\"><head>  <title>A simple webpage</title></head><body>  <h1>Simple HTML webpage</h1>  <p>Hello, world!</p></body></html>\r\n\r\n";
	ss << statusLine << headers << body << "\n";
	response = ss.str();
}

std::string Response::setHeaders(Request &req, Configuration &config, Response &resp)
{	
	// std::string headers = "Content-Type: text/html\r\nContent-Length: 153\n\n";
	// return headers;
	return "";	
}

void Response::handleGetRequest(Configuration &config)
{
	DIR*	directoryPtr;
	// = opendir(_requestLine.absolutePath.c_str());
	// if (directoryPtr == NULL) {
	// 	if (errno == ENOENT) {
	// 		_noBodyResponseDriver(404, "", false);
	// 	} else if (errno == EACCES) {
	// 		_noBodyResponseDriver(403, "", false);
	// 	} else {
	// 		_noBodyResponseDriver(500, "", false);
	// 	}
	// 	return ;
	// }
	processServerBlock(config, this->req);
	
	createResponseStr();
	// std::stringstream ss;
	// std::string headers = "Content-Type: text/html\r\nContent-Length: 153\n\n";
	// // std::string headers = setHeaders(req, config, *this);
	// std::string body = "<!DOCTYPE html><html lang=\"en\"><head>  <title>A simple webpage</title></head><body>  <h1>Simple HTML webpage</h1>  <p>Hello, world!</p></body></html>\r\n\r\n";
	// std::string statusLine = "HTTP/1.1 200 OK\n";
	// ss << statusLine << headers << body << "\n";
	// response = ss.str();
}

void Response::handlePostRequest(Configuration &config)
{
	return;
}

void Response::handleDeleteRequest(Configuration &config)
{
	return;
}
std::string Response::getResponse(Configuration &config)
{
	switch (req.getMethod())
	{
	case GET:
		handleGetRequest(config);
		break;
	case POST:
		handlePostRequest(config);
		break;
	case DELETE:
		handleDeleteRequest(config);
	// case UNKNOWN:
	// 	handleUnknownRequest();
		break;
	}
	return response;
}