#pragma once
#include "Buffer.hpp"
#include "HttpParser.hpp"
#include "http/header.hpp"
#include "http/message.hpp"
#include <cstdio>
#include <cstring>
#include <sys/select.h>

class CommonGatewayInterface
{
    int _inputFd[2], _outputFd[2];
    std::string _cgiExecName, _filepath, _data;
    int _csockFd;
    HTTP::Request* _req;
    HTTP::Header _header;

    bool _hasStarted;

    HttpParser* _parser;
    pid_t _pid;

  public:
    CommonGatewayInterface(int csockFd,
                           HTTP::Request* req,
                           const std::string& cgiExecName,
                           const std::string& filepath);
    ~CommonGatewayInterface(void);

    HTTP::Request* request(void);

    void start(void);

    HTTP::Header& header(void);

    bool parse(const char* data, size_t n);

    bool hasStarted(void) const;
    void stopParser(void);

    int getOutputFd(void) const;
    int getInputFd(void) const;
    int getClientFd(void) const;
    bool isDone(void) const;
};
