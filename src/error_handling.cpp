#include "Constants.hpp"
#include "core.hpp"
#include "http/Exception.hpp"

void
handleHttpException(HTTP::Exception& e)
{
    HTTP::Request* req = e.req();
    HTTP::Response* res = req->response();
    int fd = req->getClientFd();

    req->parser->stop();

    if (uploaders.find(fd) != uploaders.end()) {
        delete uploaders[fd];
        uploaders.erase(fd);
    }

    if (cgis.find(fd) != cgis.end()) {
        delete cgis[fd];
        cgis.erase(fd);
    }

    res->setStatus(e.status());
    res->sendFile(ERROR_PAGE);
    res->data = res->formatHeader();
    res->data += res->body;
}
