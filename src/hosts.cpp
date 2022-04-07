#include "Constants.hpp"
#include "config/ConfigParser.hpp"
#include "core.hpp"
#include "utils/Logger.hpp"
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

using std::clog;
using std::map;
using std::vector;

void
initHosts(ConfigItem* global)
{
    vector<ConfigItem*> serverBlocks = global->findBlocks("server");

    /* for each server block */

    if (serverBlocks.size() == 0) {
        std::clog << ORANGE << "/!\\ Warning "
                  << ": no server block found, the server will hang forever "
                     "and will do nothing, and that's sad :( "
                  << ORANGE << "/!\\\n\n"
                  << CLR;
    }

    for (size_t i = 0; i != serverBlocks.size(); ++i) {

        vector<ConfigItem*> listenDirs = serverBlocks[i]->findBlocks("listen");

        /* for each listen directive in the current server block */

        for (size_t j = 0; j != listenDirs.size(); ++j) {
            ListenData ldata = parseListen(listenDirs[j]->getValue());

            /* if there is no socket bound to the port */
            if (hosts.find(ldata.port) == hosts.end()) {
                sockaddr_in& sin = hosts[ldata.port].addr;
                int& ssockFd = hosts[ldata.port].ssockFd;

                memset(&sin, 0, sizeof(sin));
                sin.sin_port = htons(ldata.port);
                sin.sin_family = AF_INET;
                sin.sin_addr.s_addr = ldata.v4;

                if ((ssockFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                    perror("socket: ");
                    return;
                }

                int reuse = 1;
                socklen_t n = sizeof(reuse);
                setsockopt(ssockFd, SOL_SOCKET, SO_REUSEADDR, &reuse, n);

                if (bind(ssockFd, (sockaddr*)&sin, sizeof(sin)) == -1) {
                    perror("bind: ");
                    return;
                }

                glogger << Logger::getTimestamp() << " New host bound to port "
                        << ldata.port << "\n";

                if (listen(ssockFd, 1024) == -1) {
                    perror("listen: ");
                    return;
                }

                fcntl(ssockFd, F_SETFL, O_NONBLOCK);
                FD_SET(ssockFd, &select_rset);
            }

            /* in any case, the current block becomes a candidate for that host
             */

            hosts[ldata.port].candidates.push_back(serverBlocks[i]);
        }
    }
}

void
destroyHosts(void)
{
    for (map<uint16_t, Host>::const_iterator cit = hosts.begin();
         cit != hosts.end();
         ++cit) {
        close(cit->second.ssockFd);
    }
}
