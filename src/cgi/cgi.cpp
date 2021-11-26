#include "cgi/cgi.hpp"
#include <unistd.h>
#include <cstring>

CommonGatewayInterface::CommonGatewayInterface(HTTP::Request& req, const std::string& cgiExecName, const std::string& filepath, size_t contentLength = 0):
 _req(req), _cgiExecName(cgiExecName), _proc(-1)
{
    std::map<std::string, std::string> cgiEnv;
    std::ostringstream oss;
    pipe(_inputFd);
    pipe(_outputFd);

    _proc = fork();
    
    oss << contentLength;
    cgiEnv["CONTENT_LENGTH"] = oss.str();

    if (_proc == 0) {
        dup2(_inputFd[0], STDIN_FILENO);
        dup2(_outputFd[1], STDOUT_FILENO);

        char* const argv[3] = {
            strdup("php-cgi"),
            strdup(filepath.c_str()),
            NULL
        };

        execvpe(filepath.c_str(), argv);

        perror("execvp: ");
        exit(1);
    }
}

CommonGatewayInterface::~CommonGatewayInterface(void)
{
    close(_inputFd[0]);
    close(_inputFd[1]);
    close(_outputFd[0]);
    close(_outputFd[1]);
}