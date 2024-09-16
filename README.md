# Webserv
This project is about writing HTTP server.

https://www.notion.so/Webserv-e0b101573b614f959497d4856d97e59c

Todo List - Webserv 
	
- <s>determine the main classes or structs we’ll need throughout the project</s>
- <s>determine how we’ll divide the work</s>	
- <s>have a basic http request working (without a configuration file yet)</s>
- <s>understand how configuration files work (Mostafa) and need to be parsed</s>
- <s>understand how to listen on all clients and ports with select(), poll(0 or epoll() (Victoire)</s>
- <s>understand how CGI works and is used (Victoire)</s>
- <s>how to parse the http request and create response (Alisa)</s>
- create a map / schema for all processes (all)
- <s>add exceptions instead of returning error and exiting</s>
- <s>correctly rename all vars and functions</s>

NEW TODOS:
- Parsing:
	- client_max_body_size can be set to 0, check the syntax how it can be defined in nginx config (Sets the maximum allowed size of the client request body. If the size in a request exceeds the configured value, the 413 (Request Entity Too Large) error is returned to the client. Please be aware that browsers cannot correctly display this error. Setting size to 0 disables checking of client request body size. )
	- add the first (default) location block if there's no "location /" block set in the config
- Errors handling:
	- Proper status code everywhere (all requests) and verify that correct error pages are returned (default or custom)
	- <s>Setup default error pages (add a file with them)</s>
- Connecting config and request:
	- <s>Check host, port and server_names and have server block</s>
	- <s>Inside server block check for location path and return either corrponding location or default location</s>

- Directives:
	- <s>Server names</s>
	- <s>Error pages</s>
	- <s>Limit client body size</s>
	- <s>HTTP methods</s>
	- HTTP redirection
	- <s>Autoindex: turn on or off directory listing (add to the whole process)</s>
	- <s>The index directive (specifies the index file name(s) to be served when a directory is requested. By default, Nginx looks for an index.html file in the root directory)</s>
	- <s>Alias (Define a directory or a file from where the file should be searched (for example, if url /kapouet is rooted to /tmp/www, url /kapouet/pouic/toto/pouet is /tmp/www/pouic/toto/pouet))</s>
	- CGI (see bellow)
- CGI part:
	- add a checkif there's a cgi pass
	- run cgi code if there's a path // handle by nginx if there's no path
- Creating final response string for each request:
	- Set required headers dynamically for all requests
	- Verify that the correct body is set

- Signals to handle + valgrind checks to add

- Decide which files we want to serve to demonstrate functionality of webserv:
	- a fully static website
	- upload files functionality
	- GET, POST, and DELETE methods
	- multiple ports

**Main parts of the project:**
1. Server logic (loop)
2. HTTP request (parsing)
3. Configuration file (parsing)
4. Creating response (analyze config and request)
5. CGI
