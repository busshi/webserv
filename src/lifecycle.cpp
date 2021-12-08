#include "HttpParser.hpp"
#include "core.hpp"
#include "utils/Logger.hpp"
#include <cstring>
#include <fcntl.h>
#include <iomanip>
#include <signal.h>
#include <sys/errno.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

using std::map;

static void
handleSigint(int)
{
    isWebservAlive = false;
}

static void
handleSigpipe(int)
{
    std::cout << "Received SIGPIPE" << std::endl;
}

static void
closeConnection(int sockfd)
{
    requests.erase(sockfd);
    delete cgis[sockfd];
    cgis.erase(sockfd);
    FD_CLR(sockfd, &select_wset);
    FD_CLR(sockfd, &select_rset);
    close(sockfd);

#ifdef LOGGER
    glogger << Logger::INFO << Logger::getTimestamp()
            << " Closed connection to fd " << sockfd << "\n";
#endif
}

static void
handleCgiEvents(fd_set& rsetc, fd_set& wsetc)
{
    for (map<int, CommonGatewayInterface*>::const_iterator cit = cgis.begin();
         cit != cgis.end();
         ++cit) {
        int csockfd = cit->first;
        CommonGatewayInterface* cgi = cit->second;

        if (!cgi->hasStarted()) {
            continue;
        }

        HTTP::Request* reqp = requests[csockfd];

        // we can write to cgi output

        if (FD_ISSET(cgi->getOutputFd(), &wsetc)) {
            std::string body = reqp->body.str();

            if (!body.empty()) {
                write(cgi->getOutputFd(), body.c_str(), body.size());
                reqp->body.str("");
            }
        }

        // we have something to read from the cgi

        if (FD_ISSET(cgi->getInputFd(), &rsetc)) {
            char buf[BUFSIZE + 1];

            int ret = read(cgi->getInputFd(), buf, BUFSIZE);

            if (ret == -1) {
                perror("cgi read:");
            }

            else if (ret == 0) {
                cgi->stopParser();
            }

            else {
                buf[ret] = 0;
                cgi->parse(buf);
            }
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
            char buf[BUFSIZE + 1];
            int ret = recv(csockfd, buf, BUFSIZE, 0);

            if (ret == -1) {
#ifdef LOGGER
                glogger << Logger::INFO << Logger::getTimestamp() << " "
                        << strerror(errno) << "\n";
#endif
                closeConnection(csockfd);
                continue;
            }

            if (ret > 0) {
                buf[ret] = 0;

                reqp->parse(buf);
            }
        }

        /* if we can write */

        if (FD_ISSET(csockfd, &wsetc)) {
            HTTP::Response* resp = reqp->response();

            if (resp) {
                BinBuffer& bbuf = resp->data;

                if (!bbuf.isConsumed()) {
                    std::pair<const uint8_t*, size_t> c = bbuf.getbuf();
                    int ret = send(csockfd, c.first, c.second, 0);
                    bbuf.consume(ret);
                }

                // if we are done, then close the connection
                // TODO: specialize this behaviour to add Keep-Alive feature
                if (cgis.find(csockfd) != cgis.end()) {
                    if (cgis[csockfd]->isDone()) {
                        closeConnection(csockfd);
                    }
                }

                else if (reqp->isDone() && bbuf.isConsumed()) {
                    std::cout << "Cloclose!" << std::endl;
                    closeConnection(csockfd);
                }
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

#ifdef LOGGER
            glogger << Logger::getTimestamp() << " New connection to port "
                    << it->first << " accepted (fd=" << connection << ")\n";
#endif

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
    signal(SIGPIPE, &handleSigpipe);

    size_t dotN = 0;

    while (isWebservAlive) {
        /*std::cout << "Webserv is running" << std::left << std::setw(3)
                  << std::string(dotN, '.') << "\r" << std::flush;*/
        if (++dotN > 3) {
            dotN = 0;
        }

        fd_set rsetc = select_rset, wsetc = select_wset;

        /* according to the man, it is better to initialize it each time */
        struct timeval timeout = { .tv_sec = 1, .tv_usec = 0 };

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
