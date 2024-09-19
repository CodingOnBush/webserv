#include "Cgi.hpp"



char **createEnv(Request &req, LocationBlock &location)
{
    //let's parse the request and create the env variables
    std::string scriptName;
    std::string pathInfo;
    char **env = new char *[8];
    std::stringstream ss;
    std::string uri = req.getUri();

    if (location.pathInfo == true)
    {
        std::cout << "URI: " << uri << std::endl;
        size_t pos = uri.find(".py");
        if (pos != std::string::npos)
        {
            std::string pathInfo = uri.substr(pos + 3); // +3 to skip past ".py"            
            scriptName = uri.substr(0, pos + 3); // +3 to include ".py"
        }
    }
    else
    {
        scriptName = req.getUri();
        pathInfo = "";
    }
    ss << "CONTENT_LENGTH=" << req.getBody().size();
    env[0] = strdup(ss.str().c_str());
    ss.str("");
    ss << "CONTENT_TYPE=" << req.getHeaders()["Content-Type"];
    env[1] = strdup(ss.str().c_str());
    ss.str("");
    ss << "UPLOAD_LOCATION=" << location.uploadLocation;
    env[2] = strdup(ss.str().c_str());
    ss.str("");
    ss << "REQUEST_METHOD=" << req.getMethod();
    env[3] = strdup(ss.str().c_str());
    ss.str("");
    ss << "QUERY_STRING=" << req.getBody() << CRLF;
    env[4] = strdup(ss.str().c_str());
    ss.str("");
    ss << "SCRIPT_NAME=" << scriptName;
    env[5] = strdup(ss.str().c_str());
    ss.str("");
    ss << "PATH_INFO=" << pathInfo << CRLF;
    env[6] = strdup(ss.str().c_str());
    env[7] = NULL;
    return env;
}

void handleCGI(Configuration &Config, LocationBlock &location, Request &req, Response &res)
{
    char **env = createEnv(req, location);
    std::string cgiPathWithArgs = location.cgiParams.begin()->second;
    std::stringstream cgiOutput;
    struct stat st;
    std::cout << "CGI PATH: " << cgiPathWithArgs << std::endl;
    if ((stat(cgiPathWithArgs.c_str(), &st) != 0))
    {
        std::cerr << "file does not exist" << std::endl;
        res.setStatusCode(404);
        return;
    }
    else
    {
        if (!(st.st_mode & S_IXUSR))
        {
            std::cerr << "file is not executable" << std::endl;
            res.setStatusCode(403);
            return;
        }
    }
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        std::cerr << "pipe failed" << std::endl;
        res.setStatusCode(500);
        return;
    }
    int pid = fork();
    if (pid == -1)
    {
        std::cerr << "fork failed" << std::endl;
        res.setStatusCode(500);
        return;
    }
    else if (pid == 0)
    {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        char *args[] = {strdup(cgiPathWithArgs.c_str()), NULL};
        execve(cgiPathWithArgs.c_str(), args, env);
        exit(1);
    }
    else
    {
        close(pipefd[1]);
        char buffer[128];
        ssize_t bytesRead;
        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0)
        {
            buffer[bytesRead] = '\0';
            cgiOutput << buffer;
        }
        close(pipefd[0]);

        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
        {
            std::cout << "child exited with status: " << WEXITSTATUS(status) << std::endl;
        }
        if (cgiOutput.str().empty())
        {
            res.setStatusCode(500);
            return;
        }
        else if (WEXITSTATUS(status) != 0)
        {
            res.setStatusCode(500);
            return;
        }
        //add check here to verify that the file (cgiOutput) is finite (no infinite loops in py script)
        else
        {
            std::string output = cgiOutput.str();
            size_t pos = output.find("Content-Type: text/html");
            if (pos != std::string::npos)
            {
                output.erase(pos, std::string("Content-Type: text/html").length());
            }
            cgiOutput.str(output);
            res.setBody(cgiOutput.str());
            res.setMimeType("html");
            res.setStatusCode(200);
        }
    }
    for (int i = 0; env[i]; i++)
    {
        free(env[i]);
    }
    delete[] env;
}