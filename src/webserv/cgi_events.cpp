#include "Server.hpp"
#include <unistd.h>

/**
 * @brief Watch for data to read from the output pipe of each running CGI
 *
 * @param set The selected file descriptors
 */

void
Server::_handleCGIEvents(const fd_set& rset, const fd_set& wset)
{
    for (std::map<int, CommonGatewayInterface*>::const_iterator cit =
           _cgis.begin();
         cit != _cgis.end();) {
        CommonGatewayInterface* cgi = cit->second;

        // not started yet: most likely waiting for a chunked request
        if (!cgi->hasStarted()) {
            ++cit;
            continue;
        }

        if (FD_ISSET(cgi->getOutputFd(), &wset)) {
            HTTP::Request& req = _reqs[cit->first];
            (void)req;

            if (1) {
            }
        }

        // increment before possible item erasure
        ++cit;

        // We've got data to read from the php-cgi, now let's process it
        if (FD_ISSET(cgi->getInputFd(), &rset)) {
            cgi->stream();

            // EOF
            if (cgi->isDone()) {
                std::cout << "Connexion done" << std::endl;
                _closeConnection(cgi->getClientFd());
                delete cgi;
            }
        }
    }
}
