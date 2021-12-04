#include "Server.hpp"
#include <unistd.h>

void
Server::_handleClientEvents(const fd_set& rset, const fd_set& wset)
{
    for (std::map<int, HTTP::Request>::iterator it = _reqs.begin(); it != _reqs.end();) {
        int csockfd = it->first;
        HTTP::Request& req = it->second;

        /* BEGIN - IF CLIENT FD IS READABLE */

        if (FD_ISSET(csockfd, &rset)) {
           //std::cout << "Handle client read: " << it->first << std::endl;

            char buf[1024];
            buf[recv(csockfd, buf, 1023, 0)] = 0;

            // if parsing body as chunked we need another solution

            std::cout << "read" << std::endl;

            // parse the header
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

                    req.data.str("");

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
                    req.body.str(res.str());
                }
            }

            // if writing to the client is possible...
            if (FD_ISSET(csockfd, &wset)) {
                
                // if request is not handled by CGI, send an immediate response
                // NOTE: this is a naive and uncomplete way of handling non-CGI requests
                // Do not enter this if header is being parsed because we can't know if request will be cgi handled or not at this point
                if (_cgis.find(csockfd) == _cgis.end() && req.getState() != HTTP::Request::W4_HEADER) {
                    send(csockfd, req.body.str().c_str(), req.body.str().size(), 0);
                        
                    // increment before closing connection as it will invalidate the current iterator
                    ++it;
                     _closeConnection(csockfd);
                    continue ;
                }
            }

            /* BODY */

            if (req.getState() != HTTP::Request::W4_HEADER) {
                
                std::map<int, CommonGatewayInterface*>::const_iterator cgi =
                  _cgis.find(csockfd);

                // if request is handled by CGI AND cgi output file descriptor is ready for writing
                if (cgi != _cgis.end()) {
                    // unchunk incoming request body
                    if (req.isChunked() && req.getState() != HTTP::Request::DONE) {
                        req.parseChunk(buf);
                        if (req.getState() == HTTP::Request::DONE) {
                            std::ostringstream oss;

                            oss << req.data.str().size();
                            req.setHeaderField("Content-Length", oss.str());
                            _cgis[csockfd]->start();
                        }
                    }
                
                    if (!req.isChunked()) {
                        req.data << buf;
                    }
                }
            }
        }

        /* END - IF CLIENT FD IS READABLE */

        ++it;
    }
}