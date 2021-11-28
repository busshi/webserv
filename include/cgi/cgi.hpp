#pragma once
#include "http/message.hpp"
#include <cstdio>
#include <cstring>
#include <sys/select.h>

class CommonGatewayInterface {
    int _inputFd[2], _outputFd[2];
    std::string _cgiExecName, _filepath, _data;
    int _csockFd;
    fd_set& _fdSet;
    HTTP::Request& _req;
    pid_t _proc;
    HTTP::Response _res;
    enum { STREAMING_HEADER, STREAMING_BODY } _state;
    bool _isDone;

    public:
        CommonGatewayInterface(int csockFd, fd_set& fdSet, HTTP::Request& req, const std::string& cgiExecName, const std::string& filepath);
        ~CommonGatewayInterface(void);

        void start(void);

        int getOutputFd(void) const;
        int getInputFd(void) const;
        int getClientFd(void) const;
        bool isDone(void) const;

        const std::string& getData(void) const;

        int read(void);

        void stream(void);
};