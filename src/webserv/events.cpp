#include "logger/Logger.hpp"
#include "webserv/Server.hpp"

#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

void Server::_closeConnection(int sockFd)
{
    _reqs.erase(sockFd);
    _data.erase(sockFd);
    _clients.erase(sockFd);
    _cgis.erase(sockFd);
    FD_CLR(sockFd, &_fdset);
    close(sockFd);
}

void
Server::_handleClientEvents(const fd_set& set)
{
    std::cout << "---- Handle clients -----" << std::endl;
    for (std::set<int>::const_iterator cit = _clients.begin();
         cit != _clients.end();
         ++cit) {

        std::cout << *cit << std::endl;

        if (FD_ISSET(*cit, &set)) {

            // header hasn't been parsed we need to
            if (_reqs.find(*cit) == _reqs.end()) {
                char buf[1024] = { 0 };

                recv(*cit, buf, 1023, 0);
                _data[*cit] += buf;

                std::string::size_type pos = 0;

                if ((pos = _data[*cit].find(HTTP::BODY_DELIMITER)) !=
                    std::string::npos) {
                    // creates a new Request object and parses the header
                    HTTP::Request& req =
                      _reqs
                        .insert(std::make_pair(
                          *cit,
                          HTTP::Request(*cit, _data[*cit].substr(0, pos))))
                        .first->second;

                    // parse beginning of the body
                    req.body << _data[*cit].substr(pos, std::string::npos);

                    socklen_t slen = sizeof(sockaddr_in);
                    sockaddr_in addr;
                    getsockname(*cit, (sockaddr*)&addr, &slen);

                    uint16_t port = ntohs(addr.sin_port);

                    req.setServerBlock(_selectServer(
                      _hosts[port].candidates, req.getHeaderField("host")));


                    HTTP::Response res(*cit);

                    _createResponse(req, res, req.getServerBlock());

                    // we don't want to enter that block if request has been taken by the CGI
                    if (_cgis.find(*cit) == _cgis.end()) {
                        send(*cit, res.str().c_str(), res.str().size(), 0);

                        // we need to wait for CGI to end before closing!
                        _closeConnection(*cit);
                    }

                }
            }
        } 
    }
    std::cout << "---- END clients -----" << std::endl;
}

/**
 * @brief Watch for data to read from the output pipe of each running CGI
 * 
 * @param set The selected file descriptors
 */

void Server::_handleCGIEvents(const fd_set& set)
{
    std::cout << "cgis: " << _cgis.size() << std::endl; 
    for (std::map<int, CommonGatewayInterface*>::const_iterator cit = _cgis.begin(); cit != _cgis.end();) {
        CommonGatewayInterface* cgi = cit->second;
        ++cit;

        // We've got data to read from the php-cgi, now let's process it
        if (FD_ISSET(cgi->getOutputFd(), &set)) {
            cgi->stream();

            // EOF
            if (cgi->isDone()) {
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
Server::_handleServerEvents(const fd_set& set)
{
    for (HostMap::iterator it = _hosts.begin(); it != _hosts.end(); ++it) {

        // server socket is readable without blocking, we've got a new connection
        if (FD_ISSET(it->second.ssockFd, &set)) {
            socklen_t slen = sizeof(it->second.addr);
            int connection =
              accept(it->second.ssockFd, (sockaddr*)&it->second.addr, &slen);

            if (connection == -1) {
                perror("accept: ");
                continue ;
            }

            std::cout << "Added connection for fd " << connection << std::endl;

            fcntl(connection, F_SETFL, O_NONBLOCK);
            FD_SET(connection, &_fdset);
            _clients.insert(connection);

            glogger << "Initialized a new connection on port " << it->first << "\n";
        }
    }
}
