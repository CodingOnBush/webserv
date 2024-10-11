#include "Cgi.hpp"

std::time_t cgiStart = 0;
#define CGITIMEOUT 2

bool needsCGI(LocationBlock location, Request &req)
{
    std::string uri = req.getUri();
    if (!location.cgiExtensions.empty())
    {
        for (std::vector<std::string>::iterator it = location.cgiExtensions.begin(); it != location.cgiExtensions.end(); it++)
        {
            size_t extentionSize = (*it).size();
            if (uri.size() >= extentionSize && uri.substr(uri.size() - extentionSize) == *it && !isDirectory(location.root + uri))
            {
                return true;
            }
        }
    }
    return false;
}

void freeEnv(char **env)
{
    for (int i = 0; env[i] != NULL; i++)
    {
        free(env[i]);
    }
    delete[] env;
}
std::string createEnvVar(const std::string &key, const std::string &value)
{
    std::stringstream ss;
    ss << key << "=" << value;
    return ss.str();
}

char **createEnv(Request &req, LocationBlock &location)
{
    std::vector<std::string> envVars;
    envVars.push_back(createEnvVar("CONTENT_LENGTH", intToString(req.getBody().size())));
    envVars.push_back(createEnvVar("CONTENT_TYPE", req.getHeaders()["Content-Type"]));
    envVars.push_back(createEnvVar("UPLOAD_LOCATION", location.uploadLocation));
    envVars.push_back(createEnvVar("REQUEST_METHOD", intToString(req.getMethod())));
    envVars.push_back(createEnvVar("QUERY_STRING", req.getBody() + CRLF));

    char **env = new char *[envVars.size() + 1];
    for (size_t i = 0; i < envVars.size(); ++i)
    {
        env[i] = strdup(envVars[i].c_str());
    }
    env[envVars.size()] = NULL;
    return env;
}

void handleCGI(LocationBlock &location, Request &req, Response &res)
{
    char **env = createEnv(req, location);
    std::string cgiPathWithArgs = location.root + req.getUri();
    std::stringstream cgiOutput;
    struct stat st;
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
            freeEnv(env);
            return;
        }
    }
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        std::cerr << "pipe failed" << std::endl;
        res.setStatusCode(500);
        freeEnv(env);
        return;
    }
    int pid = fork();
    if (pid == -1)
    {
        std::cerr << "fork failed" << std::endl;
        res.setStatusCode(500);
        freeEnv(env);
        return;
    }
    else if (pid == 0)
    {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        char *args[] = {strdup(cgiPathWithArgs.c_str()), NULL};
        execve(cgiPathWithArgs.c_str(), args, env);
        exit(1);
    }
    else
    {
        close(pipefd[1]);
        char buffer[128];
        int status;
        cgiStart = std::time(0);

        while (true) 
        {
            if (std::time(0) - cgiStart > CGITIMEOUT) 
            {
                kill(pid, SIGKILL);
                res.setStatusCode(504);
                freeEnv(env);
                close(pipefd[0]);
                return;
            }
            if (waitpid(pid, &status, WNOHANG) != 0) 
                break;
        }

        ssize_t bytesRead;
        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0)
        {
            buffer[bytesRead] = '\0';
            cgiOutput << buffer;
        }
        close(pipefd[0]);
        if (cgiOutput.str().empty())
        {
            std::cerr << "empty output" << std::endl;
            res.setStatusCode(500);
            return;
        }
        else if (WEXITSTATUS(status) != 0)
        {
            std::cout << "exit error" << std::endl;
            res.setStatusCode(500);
            return;
        }
        else
        {
            std::string output = cgiOutput.str();
            size_t pos = output.find("Content-Type: text/html");
            if (pos != std::string::npos)
                output.erase(pos, std::string("Content-Type: text/html").length());
            cgiOutput.str(output);
            res.setBody(cgiOutput.str());
            res.setMimeType("html");
            res.setStatusCode(200);
        }
    }
    freeEnv(env);
}
