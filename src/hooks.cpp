#include "core.hpp"
#include "http/message.hpp"
#include "utils/Logger.hpp"
#include <iostream>
#include <string>
#include <unistd.h>

#define GET_REQ(loc) *reinterpret_cast<HTTP::Request*>(loc);

using std::string;

void
onHeader(const string& method,
         const string& loc,
         const string& protocol,
         uintptr_t requestLoc)
{
    HTTP::Request& req = GET_REQ(requestLoc);

    req.setMethod(method);
    req.setLocation(loc);
    req.setProtocol(protocol);
}

void
onHeaderField(const string& name, const string& value, uintptr_t requestLoc)
{
    HTTP::Request& req = GET_REQ(requestLoc);

    req.setHeaderField(name, value);
}

void
onHeaderParsed(uintptr_t requestLoc)
{
    HTTP::Request& req = GET_REQ(requestLoc);

    // get port to which the connection is addressed
    socklen_t slen = sizeof(sockaddr_in);
    sockaddr_in addr;
    getsockname(req.getClientFd(), (sockaddr*)&addr, &slen);
    uint16_t port = ntohs(addr.sin_port);

    ConfigItem* serverBlock =
      selectServer(hosts[port].candidates, req.getHeaderField("host"));

    req.setServerBlock(serverBlock);

    HTTP::Response& res = *req.createResponse();

    createResponse(req, res, serverBlock);

    res.data = res.data.append(res.formatHeader()) + res.body;
}

void
onBodyFragment(const string& fragment, uintptr_t requestLoc)
{
    HTTP::Request& req = GET_REQ(requestLoc);

    req.body << fragment;
}

void
onBodyChunk(const string& chunk, uintptr_t requestLoc)
{
    HTTP::Request& req = GET_REQ(requestLoc);

    req.body << chunk;
}

void
onBodyUnchunked(uintptr_t requestLoc)
{
    HTTP::Request& req = GET_REQ(requestLoc);

    int csockfd = req.getClientFd();

    if (cgis.find(csockfd) != cgis.end() && !cgis[csockfd]->hasStarted()) {
        std::ostringstream oss;

        oss << req.body.str().size();
        req.header().setField("Content-Length", oss.str());
        cgis[csockfd]->start();
    }

    (void)req;
}

void
onBodyParsed(uintptr_t requestLoc)
{
    HTTP::Request& req = GET_REQ(requestLoc);

    (void)req;
}
