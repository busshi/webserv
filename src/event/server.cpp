#include "core.hpp"
#include "http/message.hpp"
#include <fcntl.h>
#include <sys/socket.h>

using HTTP::MessageParser;
using HTTP::Request;
using std::make_pair;
using std::map;

void
handleServerEvents(const MessageParser::Config& parserConf, fd_set& rsetc)
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
                    << it->first << " accepted (fd=" << connection << ")\n";

            fcntl(connection, F_SETFL, O_NONBLOCK);

            Request* req = new Request(connection, parserConf);
            requests.insert(make_pair(connection, req));
        }
    }
}
