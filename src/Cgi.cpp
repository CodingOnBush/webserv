#include "Cgi.hpp"

void printCgiParams(const std::map<std::string, std::string>& cgiParams) {
    for (const auto& param : cgiParams) {
        std::cout << param.first << ": " << param.second << std::endl;
    }
}

void handleCGI(Configuration &Config, LocationBlock &location, Request &req, Response &res)
{
	//create environment
	//run script
	//get output
	//set output as body
	//set status code
	//set mime type
	//set headers
	//set status line
	//set response
	printCgiParams(location.cgiParams);
	std::string cgiPathWithArgs = "./www/cgi-bin/test.py";
	std::string cgiOutput;
	std::cout << "CGI PATH: " << cgiPathWithArgs << std::endl;
	FILE *fp = popen(cgiPathWithArgs.c_str(), "r");
	if (fp == NULL)
	{
		res.setStatusCode(500);
		return;
	}
	char buf[128];
	while (fgets(buf, sizeof(buf), fp) != NULL)
	{
		cgiOutput += buf;
	}
	pclose(fp);
	res.setBody(cgiOutput);
	res.setMimeType("html");
	res.setStatusCode(200);
};