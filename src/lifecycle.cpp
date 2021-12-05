#include "HttpParser.hpp"
#include "core.hpp"
#include "logger/Logger.hpp"
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
listenToClientEvents(fd_set& rsetc)
{
    for (map<int, HTTP::Request*>::const_iterator cit = requests.begin();
         cit != requests.end();
         ++cit) {
        if (FD_ISSET(cit->first, &rsetc)) {
            HTTP::Request* reqp = cit->second;
            char buf[1024];

            recv(cit->first, buf, 1023, 0);
            reqp->parse(buf);
        }
    }
}

static void
listenToServerEvents(const HttpParser::Config& parserConf,
                     fd_set& rsetc,
                     fd_set& rset,
                     fd_set& wset)
{
    (void)parserConf;

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

            FD_SET(connection, &rset);
            FD_SET(connection, &wset);
            requests.insert(std::make_pair(
              connection, new HTTP::Request(connection, parserConf)));
        }
    }
}

void
lifecycle(const HttpParser::Config& parserConf, fd_set& rset, fd_set& wset)
{
    signal(SIGINT, &handleSigint);

    while (isWebservAlive) {
        fd_set rsetc = rset, wsetc = wset;

        /* according to the man, it is better to initialize it each time */
        struct timeval timeout = { .tv_sec = 6, .tv_usec = 0 };

        int nready = select(FD_SETSIZE, &rsetc, &wsetc, 0, &timeout);

        if (nready == -1) {
            perror("select: ");
            return;
        }

        listenToServerEvents(parserConf, rsetc, rset, wset);
        listenToClientEvents(rsetc);

        /* events will take place here */
    }
}
