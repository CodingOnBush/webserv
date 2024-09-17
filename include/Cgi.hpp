#pragma once
#include "Configuration.hpp"
#include "Request.hpp"
#include "Response.hpp"

class Response;

void handleCGI(Configuration &Config, LocationBlock &location, Request &req, Response &res);
void printCgiParams(const std::map<std::string, std::string>& cgiParams);
