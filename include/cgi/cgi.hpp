#pragma once
#include "http/message.hpp"
#include <cstdio>

class CommonGatewayInterface {
    int _inputFd[2], _outputFd[2];
    HTTP::Request _req;
    std::string _cgiExecName;
    pid_t _proc;

    char** _mapToEnviron(const std::map<std::string, std::string>& m)
    {
        char** envp = new char*[m.size() + 1], **p = envp;

        for (std::map<std::string, std::string>::const_iterator cit = m.begin(); cit != m.end(); ++cit) {
            *p = strdup((cit->first + "=" + cit->second).c_str());
        }

        return envp;
    }

    public:
        CommonGatewayInterface(HTTP::Request& req, const std::string& cgiExecName, const std::string& filepath, size_t contentLength = 0);
        ~CommonGatewayInterface(void);

        CommonGatewayInterface& passFile(const std::string& filepath);

        

};