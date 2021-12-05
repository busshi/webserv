#include "HttpParser.hpp"
#include "core.hpp"
#include "logger/Logger.hpp"
#include <fcntl.h>
#include <signal.h>
#include <sys/select.h>
#include <unistd.h>

using std::map;

static void
handleSigint(int)
{
    isWebservAlive = false;
}

static void
closeConnection(int sockfd)
{
    requests.erase(sockfd);
    cgis.erase(sockfd);
    FD_CLR(sockfd, &select_wset);
    FD_CLR(sockfd, &select_rset);
    close(sockfd);
}

static void
handleCgiEvents(fd_set& rsetc, fd_set& wsetc)
{
    for (map<int, CommonGatewayInterface*>::const_iterator cit = cgis.begin();
         cit != cgis.end();
         ++cit) {
        int csockfd = cit->first;
        CommonGatewayInterface* cgi = cit->second;
        HTTP::Request* reqp = requests[csockfd];

        // we can write to cgi output

        if (FD_ISSET(cgi->getOutputFd(), &wsetc)) {
            std::string body = reqp->body.str();

            if (!body.empty()) {
                write(cgi->getOutputFd(), body.c_str(), body.size());
            }
        }

        // we have something to read from the cgi

        if (FD_ISSET(cgi->getInputFd(), &rsetc)) {
        }
    }
}

static void
handleClientEvents(fd_set& rsetc, fd_set& wsetc)
{
    for (map<int, HTTP::Request*>::const_iterator cit = requests.begin();
         cit != requests.end();) {
        int csockfd = cit->first;
        HTTP::Request* reqp = cit->second;

        ++cit;

        /* if there is something to read */

        if (FD_ISSET(csockfd, &rsetc)) {
            char buf[1024];

            recv(csockfd, buf, 1023, 0);
            reqp->parse(buf);
        }

        /* if we can write */

        if (FD_ISSET(csockfd, &wsetc)) {
            if (reqp->isDone()) {
                std::string resData = reqp->response()->str();

                send(csockfd, resData.c_str(), resData.size(), 0);

                closeConnection(csockfd);
            }
        }
    }
}

static void
handleServerEvents(const HttpParser::Config& parserConf, fd_set& rsetc)
{
    for (map<uint16_t, Host>::iterator it = hosts.begin(); it != hosts.end();
         ++it) {

        if (FD_ISSET(it->second.ssockFd, &rsetc)) {

            socklen_t slen = sizeof(it->second.addr);
            int connection =
              accept(it->second.ssockFd, (sockaddr*)&it->second.addr, &slen);

            if (connection == -1) {
                perror("accept: ");
                continue;
            }

            glogger << Logger::getTimestamp() << " New connection to port "
                    << it->first << " accepted\n";

            fcntl(connection, F_SETFL, O_NONBLOCK);
            FD_SET(connection, &select_rset);
            FD_SET(connection, &select_wset);

            requests.insert(std::make_pair(
              connection, new HTTP::Request(connection, parserConf)));
        }
    }
}

void
lifecycle(const HttpParser::Config& parserConf)
{
    signal(SIGINT, &handleSigint);

    while (isWebservAlive) {
        fd_set rsetc = select_rset, wsetc = select_wset;

        /* according to the man, it is better to initialize it each time */
        struct timeval timeout = { .tv_sec = 6, .tv_usec = 0 };

        int nready = select(FD_SETSIZE, &rsetc, &wsetc, 0, &timeout);

        if (nready == -1) {
            perror("select: ");
            return;
        }

        handleClientEvents(rsetc, wsetc);
        handleServerEvents(parserConf, rsetc);
        handleCgiEvents(rsetc, wsetc);
    }
}
