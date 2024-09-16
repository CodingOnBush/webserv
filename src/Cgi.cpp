#include "Cgi.hpp"

void handleCGI(Configuration &Config, LocationBlock &location, Request &req, Response &res)
{
	
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