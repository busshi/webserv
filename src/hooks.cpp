#include "Buffer.hpp"
#include "core.hpp"
#include "http/Exception.hpp"
#include "http/message.hpp"
#include "http/method.hpp"
#include "http/status.hpp"
#include "utils/Logger.hpp"
#include "utils/string.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>

#define GET_REQ(loc) *reinterpret_cast<HTTP::Request*>(loc);

using std::string;

using HTTP::isMethodImplemented;

void
onHeader(const string& method,
         const string& loc,
         const string& protocol,
         uintptr_t requestLoc)
{
    HTTP::Request& req = GET_REQ(requestLoc);

    std::string trimmedLoc = loc.size() > 1 ? trimTrailing(loc, "/") : loc;

    req.setOriginalLocation(trimmedLoc);
    req.setLocation(trimmedLoc);
    req.setProtocol(protocol);

    if (!isMethodImplemented(method)) {
        req.setMethod("UNKNOWN");
        throw HTTP::Exception(
          &req, HTTP::NOT_IMPLEMENTED, "Method not implemented by webserv");
    } else {
        req.setMethod(method);
    }

    if (!equalsIgnoreCase(protocol, "HTTP/1.1")) {
        throw HTTP::Exception(&req,
                              HTTP::HTTP_VERSION_NOT_SUPPORTED,
                              "webserv exclusively supports HTTP/1.1");
    }
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
    HTTP::Response& res = *req.response();

    processRequest(&req);

    if (cgis.find(req.getClientFd()) == cgis.end() &&
        uploaders.find(req.getClientFd()) == uploaders.end()) {
        res.data += res.formatHeader();
    }

    res.data += res.body;
}

void
onBodyFragment(const Buffer<>& fragment, uintptr_t requestLoc)
{
    HTTP::Request& req = GET_REQ(requestLoc);

    if (uploaders.find(req.getClientFd()) != uploaders.end()) {
        uploaders[req.getClientFd()]->parseFormDataFragment(
          reinterpret_cast<const char*>(fragment.raw()), fragment.size());
    } else {
        req.body += fragment;
    }
}

void
onBodyChunk(const Buffer<>& chunk, uintptr_t requestLoc)
{
    HTTP::Request& req = GET_REQ(requestLoc);

    req.body += chunk;
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
