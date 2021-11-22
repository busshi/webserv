#include "Server.hpp"
#include "Constants.hpp"
#include <iostream>
#include <signal.h>
#include <cstdlib>
#include "logger/Logger.hpp"
#include "net/socket.hpp"
#include <sstream>
#include <typeinfo>
#include <algorithm>
#include "http/message.hpp"

Logger glogger("logs", "log.txt");
bool isWebservAlive = true;

int
main(void)                  
{  
  Net::ServerSocket *ssocket1 = new Net::ServerSocket(), *ssocket2 = new Net::ServerSocket();
    ssocket1->open(); ssocket2->open();
    ssocket1->bind(8080).listen(1024);
    ssocket2->bind(8082).listen();

    std::map<Net::ClientSocket*, std::string> data;
    std::map<Net::ClientSocket*, HTTP::Request> incomingRequests;

    Net::SocketSet sockset;

    while (1) {
        (sockset += *ssocket1) += *ssocket2;
        std::list<Net::Socket*> ready = sockset.select();

        std::cout << ready.size() << " socket(s) are ready" << std::endl;

        for (std::list<Net::Socket*>::iterator cit = ready.begin(); cit != ready.end(); ++cit) {
                Net::ClientSocket* csock = dynamic_cast<Net::ClientSocket*>(*cit);

                // if this is a client socket, we need to ready the data from it to build the request
                if (csock) {

                    // parsing request header
                    if (incomingRequests.find(csock) == incomingRequests.end()) {
                        std::cout << data[csock] << std::endl; 
                       
                        data[csock] += csock->recv();

                        if (data[csock].find(HTTP::BODY_DELIMITER) != std::string::npos) {
                            incomingRequests.insert(std::make_pair(csock, HTTP::Request(data[csock])));
                        }
                    }

                    // parsing request body
                    if (incomingRequests.find(csock) != incomingRequests.end()) {
                        HTTP::Request req = incomingRequests[csock];
                        std::istringstream oss(req.getHeaderField("Content-Length"));
                        size_t contentLength = 0;
                        oss >> contentLength;

                        std::cout << "Length: " << contentLength << "\n"; 

                        if (req.body.str().size() < contentLength) {
                            std::cout << contentLength - req.body.str().size() << std::endl;
                            std::string s = csock->recv(contentLength - req.body.str().size());
                            req.body << s;
                            std::cout << "size: " << req.body.str().size() << std::endl;
                        }

                        if (req.body.str().size() >= contentLength) {
                            HTTP::Response res(req);

                             // socket cleanup
                            incomingRequests.erase(csock);
                            data.erase(csock);

                            // HERE!!!!! WE ARE SENDING THE ACTUAL RESPONSE!
                            if (req.body.str().empty()) {
                                res.send("Hello world");
                            } else {
                                res.send(req.body.str());
                            }

                            csock->send(res.str());

                            sockset -= *csock;
                            csock->close();
                            delete csock;
                        }

                    }

                // if this is a server socket, then we need to accept a new connection
                } else {
                    Net::ServerSocket* ssock = dynamic_cast<Net::ServerSocket*>(*cit);

                    // add the new client to the socket list
                    Net::ClientSocket* csock = new Net::ClientSocket(ssock->waitForConnection());
                    sockset += *csock;
                }
        }
    }

    delete ssocket1; delete ssocket2;
}