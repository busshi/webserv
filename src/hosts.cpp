#include "core.hpp"
#include "logger/Logger.hpp"
#include "webserv/config-parser/ConfigParser.hpp"
#include <cstring>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

using std::map;
using std::vector;

void
initHosts(ConfigItem* global)
{
    vector<ConfigItem*> serverBlocks = global->findBlocks("server");

    /* for each server block */

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

            hosts[ldata.port].candidates.push_back(serverBlocks[j]);
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
