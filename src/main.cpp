#include "Server.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include "../include/Request.hpp"

int main(int ac, char **av)
{
	(void)ac;
	(void)av;
	Server server;
	
	std::vector<int> ports;
	ports.push_back(8080);
	ports.push_back(8081);
	ports.push_back(8082);

	// server.setResponse("HTTP/1.1 200 OK\n");
	// server.setResponse("Content-Type: text/html\r\n");
	// server.setResponse("Content-Length: 13\n\n");
	// server.setResponse("Hello World !\r\n\r\n");
	 server.setResponse(
        "HTTP/1.1 200 OK\nContent-Type: text/html\r\nContent-Length: 55743\r\n"
        "Connection: keep-alive\nCache-Control: s-maxage=300, public, max-age=0\n\n"
        "Content-Language: en-US\nDate: Thu, 06 Dec 2018 17:37:18 GMT\nServer: meinheld/0.6.1\n\n"
        "Strict-Transport-Security: max-age=63072000\nX-Content-Type-Options: nosniff\n\n"
        "X-Frame-Options: DENY\nX-XSS-Protection: 1; mode=block\nVary: Accept-Encoding,Cookie\n\n"
        "\n\n<!DOCTYPE html><html lang=\"en\"><head>  <title>A simple webpage</title></head>"
        "<body>  <h1>Simple HTML webpage</h1>  <p>Hello, world!</p></body></html>\r\n\r\n"
    );
	server.startServer(ports);
    return 0;
}
