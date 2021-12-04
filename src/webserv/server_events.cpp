#include "Server.hpp"
#include "logger/Logger.hpp"
#include <unistd.h>

void
Server::_handleServerEvents(const fd_set& rset, const fd_set& wset)
{
    (void)wset;

    for (HostMap::iterator it = _hosts.begin(); it != _hosts.end(); ++it) {

        // server socket is readable without blocking, we've got a new connection
        if (FD_ISSET(it->second.ssockFd, &rset)) {
            socklen_t slen = sizeof(it->second.addr);
            int connection =
              accept(it->second.ssockFd, (sockaddr*)&it->second.addr, &slen);

            if (connection == -1) {
                perror("accept: ");
                continue;
            }

            std::cout << "Added connection for fd " << connection << std::endl;

            //fcntl(connection, F_SETFL, O_NONBLOCK);
            FD_SET(connection, &_rset);
            FD_SET(connection, &_wset);
            _reqs.insert(std::make_pair(connection, HTTP::Request(connection)));

            glogger << "Initialized a new connection on port " << it->first
                    << "\n";
        }
    }
}