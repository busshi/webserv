#pragma once
#include "core.hpp"

#define LP_CLOSE_CON(sockfd)                                                   \
    closeConnection(sockfd, false);                                            \
    continue;

extern char eventBuf[BUFSIZE];

void
closeConnection(int sockfd, bool keepAlive = true);

void
terminateRequest(HTTP::Request* req);

void
handleClientEvents(fd_set& rsetc,
                   fd_set& wsetc,
                   unsigned long long requestTimeout);

void
handleCgiEvents(fd_set& rsetc, fd_set& wsetc);

void
handleUploadEvents(void);

void
handleServerEvents(const HTTP::MessageParser::Config& parserConf,
                   fd_set& rsetc);
