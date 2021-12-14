#include "Constants.hpp"
#include "core.hpp"
#include "http/Exception.hpp"
#include "http/status.hpp"
#include "utils/ErrorPageGenerator.hpp"
#include "utils/string.hpp"

void
handleHttpException(HTTP::Exception& e)
{
    HTTP::Request* req = e.req();
    HTTP::Response* res = req->response();
    int fd = req->getClientFd();

    // do not keep alive
    req->setHeaderField("Connection", "close");

    req->parser->stop();

    if (uploaders.find(fd) != uploaders.end()) {
        delete uploaders[fd];
        uploaders.erase(fd);
    }

    if (cgis.find(fd) != cgis.end()) {
        delete cgis[fd];
        cgis.erase(fd);
    }

    ErrorPageGenerator gen;

    gen.checkErrorPage(
      "", ntos(e.status()), HTTP::toStatusCodeString(e.status()), e.what());

    res->setStatus(e.status());
    res->sendFile(ERROR_PAGE);
    res->data = res->formatHeader();
    res->data += res->body;
}
