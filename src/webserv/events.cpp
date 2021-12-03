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

void
Server::_handleClientEvents(const fd_set& rset, const fd_set& wset)
{
    for (std::map<int, HTTP::Request>::iterator it = _reqs.begin(); it != _reqs.end();) {
        int csockfd = it->first;
        HTTP::Request& req = it->second;

        // if there is something to read...
        if (FD_ISSET(csockfd, &rset)) {
            char buf[1024] = { 0 };
            int ret = recv(csockfd, buf, 1023, 0);

            if (req.getState() == HTTP::Request::W4_HEADER) {
                req.data << buf;

                std::string::size_type pos = 0;

                if ((pos = req.data.str().find(HTTP::BODY_DELIMITER)) !=
                    std::string::npos) {
                    
                    // remove parsed header data from the read buffer
                    strcpy(
                      buf,
                      req.data.str().substr(pos + 4, std::string::npos).c_str()
                    );

                    // actual parsing of the header
                    req.parseHeaderFromData();

                    // get port to which the connection is addressed
                    socklen_t slen = sizeof(sockaddr_in);
                    sockaddr_in addr;
                    getsockname(csockfd, (sockaddr*)&addr, &slen);
                    uint16_t port = ntohs(addr.sin_port);
                
                    ConfigItem* serverBlock = _selectServer(_hosts[port].candidates, req.getHeaderField("host"));
                    req.setServerBlock(serverBlock);

                    req.remContentLength =
                      parseInt(req.header().getField("Content-Length"), 10);

                    HTTP::Response res(csockfd);

                    _createResponse(req, res, req.getServerBlock());
                    req.data.str(res.str());
                }
            }

            // if writing to the client is possible...
            if (FD_ISSET(csockfd, &wset)) {

                if (_cgis.find(csockfd) == _cgis.end()) {
                    send(csockfd, req.data.str().c_str(), req.data.str().size(), 0);
                        
                    // increment before closing connection as it will invalidate the current iterator
                    ++it;
                     _closeConnection(csockfd);
                    continue ;
                }
            }

            // in case incoming data is from body
            if (req.getState() == HTTP::Request::W4_BODY &&
                req.remContentLength > 0) {
                std::map<int, CommonGatewayInterface*>::const_iterator cgi =
                  _cgis.find(csockfd);

                // pass the body to cgi's stdin.
                if (cgi != _cgis.end()) {
                    write(cgi->second->getOutputFd(), buf, strlen(buf));
                    req.remContentLength -= ret;
                }

                // TODO: handle body for non-CGI requests
            }
        }
        ++it;
    }
}

/**
 * @brief Watch for data to read from the output pipe of each running CGI
 *
 * @param set The selected file descriptors
 */

void
Server::_handleCGIEvents(const fd_set& rset, const fd_set& wset)
{
    (void) wset;
    
    for (std::map<int, CommonGatewayInterface*>::const_iterator cit =
           _cgis.begin();
         cit != _cgis.end();) {
        CommonGatewayInterface* cgi = cit->second;
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

/**
 * @brief Watch for new connections
 *
 * @param set
 */

void
Server::_handleServerEvents(const fd_set& rset, const fd_set& wset)
{
    (void)wset;

    for (HostMap::iterator it = _hosts.begin(); it != _hosts.end(); ++it) {

        // server socket is readable without blocking, we've got a new
        // connection
        if (FD_ISSET(it->second.ssockFd, &rset)) {
            socklen_t slen = sizeof(it->second.addr);
            int connection =
              accept(it->second.ssockFd, (sockaddr*)&it->second.addr, &slen);

            if (connection == -1) {
                perror("accept: ");
                continue;
            }

            std::cout << "Added connection for fd " << connection << std::endl;

            fcntl(connection, F_SETFL, O_NONBLOCK);
            FD_SET(connection, &_rset);
            FD_SET(connection, &_wset);
            _reqs.insert(std::make_pair(connection, HTTP::Request(connection)));

            glogger << "Initialized a new connection on port " << it->first
                    << "\n";
        }
    }
}
