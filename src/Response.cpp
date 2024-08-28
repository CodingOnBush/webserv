#include "Response.hpp"

void Response::setStatusLine()
{
	statusLine = "HTTP/1.1 200 OK\n";
	// statusLine += req.getVersion() + " ";
	// statusLine += std::to_string(resp.status) + " ";
	// statusLine += getStatusMessage();
}

void Response::setBody(std::string const &body)
{
	this->body = body;
}

void Response::setResponse()
{
	response = statusLine;
	// response += getHeaders();
	response += body;
}
void Response::handleGetRequest(Request &req, Configuration &config)
{
	(void)req;
	(void)config;
	setStatusLine();
	// std::string headers = setHeaders(req, config, resp);
	setBody("<!DOCTYPE html><html lang=\"en\"><head>  <title>A simple webpage</title></head>"
        "<body>  <h1>Simple HTML webpage</h1>  <p>Hello, world!</p></body></html>\r\n\r\n");
	setStatusLine();
	setResponse();
}

void Response::handlePostRequest(Request &req, Configuration &config)
{
	(void)req;
	(void)config;
	return;
}

void Response::handleDeleteRequest(Request &req, Configuration &config)
{
	(void)req;
	(void)config;
	return;
}
std::string Response::getResponse(Request &req, Configuration &config)
{
	switch (req.getMethod())
	{
	case GET:
		handleGetRequest(req, config);
		break;
	case POST:
		handlePostRequest(req, config);
		break;
	case DELETE:
		handleDeleteRequest(req, config);
		break;
	}
	return response;
}