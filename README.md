# Webserv
This project is about writing HTTP server.

https://www.notion.so/Webserv-e0b101573b614f959497d4856d97e59c

Todo List - Webserv 
	
- determine the main classes or structs we’ll need throughout the project
- <s>determine how we’ll divide the work</s>	
- <s>have a basic http request working (without a configuration file yet)</s>
- understand how configuration files work (Mostafa) and need to be parsed
- understand how to listen on all clients and ports with select(), poll(0 or epoll() (Victoire)
- understand how CGI works and is used (Victoire)
- how to parse the http request and create response (Alisa)
- create a map / schema for all processes (all)
- add exceptions instead of returning error and exiting
- correctly rename all vars and functions

NEW TODOS:
- Set headers dynamically
- Proper status code everywhere (all requests)
- Setup the server_names or not, check how does server_name work (add check ?)
- Setup default error pages (add check)
- Limit client body size (add check)
- HTTP methods (add check)
- Turn on or off directory listing
- Decide which files we want to serve to demonstrate functionality of webserv
- CGI part with dynamic content


Project parts:

1. Server logic
2.  HTTP response and request
3.  Configuration file (parsing)
4.  CGI
