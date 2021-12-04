#include "logger/Logger.hpp"
#include "webserv/Server.hpp"

#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>

void
Server::_closeConnection(int sockFd)
{
    _reqs.erase(sockFd);
    _cgis.erase(sockFd);
    FD_CLR(sockFd, &_rset);
    FD_CLR(sockFd, &_wset);
    close(sockFd);
}