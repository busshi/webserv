#include "core.hpp"
#include "http/message.hpp"

#include <unistd.h>

using HTTP::Request;

void
closeConnection(int sockfd, bool keepAlive = true)
{
    Request* req = requests[sockfd];

    if (uploaders.find(sockfd) != uploaders.end()) {
        delete uploaders[sockfd];
        uploaders.erase(sockfd);
    }

    if (cgis.find(sockfd) != cgis.end()) {
        delete cgis[sockfd];
        cgis.erase(sockfd);
    }

    if (keepAlive) {
        req->parser->reset();
        req->header().clear();
        req->body.clear();
        req->timer.reset();
        req->dropFile();

        delete req->response();
        req->createResponse();
    } else {
        delete req;
        requests.erase(sockfd);

        FD_CLR(sockfd, &select_wset);
        FD_CLR(sockfd, &select_rset);
        close(sockfd);
    }

    glogger << Logger::INFO << Logger::getTimestamp()
            << " Closed connection to fd " << sockfd << "\n";
}

void
terminateRequest(Request* req)
{
    req->log(std::cout) << std::endl;
    closeConnection(
      req->getClientFd(),
      !equalsIgnoreCase(req->getHeaderField("Connection"), "close"));
}
