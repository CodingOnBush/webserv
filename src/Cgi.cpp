#include "Cgi.hpp"

// void printCgiParams(const std::map<std::string, std::string>& cgiParams) {
//     for (const auto& param : cgiParams) {
//         std::cout << param.first << ": " << param.second << std::endl;
//     }
// }

// we need to merge
// we need to be clear with what's inside env

// ENV
/*
method type : GET, POST, DELETE
body : body of the request
// query string : query string of the request ?? maybe no time
content length : length of the body
upload location : location of the file to upload
*/


	// create char **env from the request with vars
	// use excve (pass env) in a fork to run the script and be able to receive the exit status (error, success, or more)
	// python script

static const char	**createEnv(Request &req, LocationBlock &location)
{
    std::map<std::string, std::string>  headers = req.getHeaders();
	const char	**env = new const char*[5];
    std::stringstream   ss;
    
    // std::cout << "CONTENT_TYPE: " << headers["Content-Type"] << std::endl;
    // std::cout << "UPLOAD_LOCATION: " << location.uploadLocation << std::endl;
    // std::cout << "REQUEST_METHOD: " << req.getMethod() << std::endl;
    // std::cout << "CONTENT_LENGTH: " << req.getBody().size() << std::endl;
	
    // ss.str("");
    // ss << "REQUEST_METHOD=" << req.getMethod();
    // // env[0] = strdup(ss.str().c_str());
    env[0] = "REQUEST_METHOD=GET";

    // ss.str("");
    // ss << "CONTENT_LENGTH=" << req.getBody().size();
	// env[1] = strdup(ss.str().c_str());
	env[1] = "CONTENT_LENGTH=0";
    
    // ss.str("");
    // ss << "CONTENT_TYPE=" << headers["Content-Type"];
    // env[2] = strdup(ss.str().c_str());
    env[2] = "CONTENT_TYPE=text/html";

    // ss.str("");
    // ss << "UPLOAD_LOCATION=" << location.uploadLocation;
    // env[3] = strdup(ss.str().c_str());
    env[3] = "UPLOAD_LOCATION=www/upload";
	env[4] = NULL;

	for (int j = 0; env[j]; j++)
	{
		std::cout << "env[" << j << "] : " << env[j] << std::endl;
	}

	return (env);
}

void handleCGI(Configuration &config, LocationBlock &location, Request &req, Response &res)
{
    // printCgiParams(location.cgiParams);
    // const char    **env;

    // env = createEnv(req, location);
    // std::string cgiPathWithArgs = "./www/cgi-bin/test.py";
    // std::stringstream cgiOutput;
    // std::cout << "CGI PATH: " << cgiPathWithArgs << std::endl;

    // int pipefd[2];
    // if (pipe(pipefd) == -1)
    // {
    //     std::cerr << "pipe failed" << std::endl;
    //     res.setStatusCode(500);
    //     return;
    // }

    // int pid = fork();

    // if (pid == -1)
    // {
    //     std::cerr << "fork failed" << std::endl;
    //     res.setStatusCode(500);
    //     return;
    // }
    // else if (pid == 0)
    // {
    //     // Child process
    //     close(pipefd[0]); // Close unused read end
    //     dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to pipe
    //     dup2(pipefd[1], STDERR_FILENO); // Redirect stderr to pipe
    //     close(pipefd[1]); // Close write end after duplicating

    //     char *args[] = {strdup(cgiPathWithArgs.c_str()), NULL};
    //     execve(cgiPathWithArgs.c_str(), args, env);
    //     exit(1); // execve failed
    // }
    // else
    // {
    //     // Parent process
    //     close(pipefd[1]); // Close unused write end
    //     char buffer[128];
    //     ssize_t bytesRead;
    //     while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0)
    //     {
    //         buffer[bytesRead] = '\0';
    //         cgiOutput << buffer;
    //     }
    //     close(pipefd[0]); // Close read end

    //     int status;
    //     waitpid(pid, &status, 0);
    //     if (WIFEXITED(status))
    //     {
    //         std::cout << "child exited with status: " << WEXITSTATUS(status) << std::endl;
    //     }

    //     res.setBody(cgiOutput.str()); // create a parsing function to parse the output and set correctly body and headers
    //     res.setMimeType("html");
    //     res.setStatusCode(200);
    // }


    res.setBody("CGI not implemented yet");
    res.setMimeType("html");
    res.setStatusCode(200);

    // delete[] env;
}
