# Webserv
This project is about writing HTTP server.

https://www.notion.so/Webserv-e0b101573b614f959497d4856d97e59c

Todo List - Webserv 
	
- <s>determine the main classes or structs we’ll need throughout the project</s>
- <s>determine how we’ll divide the work</s>	
- <s>have a basic http request working (without a configuration file yet)</s>
- understand how configuration files work (Mostafa) and need to be parsed
- <s>understand how to listen on all clients and ports with select(), poll(0 or epoll() (Victoire)</s>
- understand how CGI works and is used (Victoire)
- how to parse the http request and create response (Alisa)
- create a map / schema for all processes (all)
- add exceptions instead of returning error and exiting
- correctly rename all vars and functions

NEW TODOS:
- <s>Set headers dynamically</s>
- Proper status code everywhere (all requests)
- Check host, port and server_names and have server block
- Inside server block check for location path and return either corrponding location or default location

- Limit client body size (add to the whole process)
- HTTP methods (add to the whole process)
- Turn on or off directory listing (add to the whole process)

- Setup default error pages (add a file with them)
- Decide which files we want to serve to demonstrate functionality of webserv
- CGI part with dynamic content


Project parts:

1. Server logic
2. HTTP response and request
3. Configuration file (parsing)
4. CGI
