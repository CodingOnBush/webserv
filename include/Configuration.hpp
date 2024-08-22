#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

/*
# Configuration File’s Structure
nginx consists of modules which are controlled by directives specified 
in the configuration file. Directives are divided into simple directives 
and block directives. A simple directive consists of the name and 
parameters separated by spaces and ends with a semicolon (;). 
A block directive has the same structure as a simple directive, 
but instead of the semicolon it ends with a set of additional 
instructions surrounded by braces ({ and }). If a block directive can 
have other directives inside braces, it is called a 
context (examples: server or location).

The rest of a line after the # sign is considered a comment.
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

struct ServerStruct {
	std::string server_name;
	std::string host;
	int 		port;
	std::string root;
	std::string index;
	std::string error_page;
	std::string location;
	std::string autoindex;
	std::string client_max_body_size;
	std::string server_tokens;
	std::string keepalive_timeout;
	std::string keepalive_requests;
};

class Configuration
{
private:
	std::vector<ServerStruct> servers;
public:
	Configuration(std::string const &filename);
	~Configuration();
};

#endif // CONFIGURATION_HPP