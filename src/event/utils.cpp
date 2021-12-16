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
        req->clear();
    } else {
        delete req;
        requests.erase(sockfd);
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
