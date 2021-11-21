#include "Server.hpp"
#include "Constants.hpp"
#include <iostream>
#include <signal.h>
#include <cstdlib>
#include "logger/Logger.hpp"
#include "net/socket.hpp"
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

    Net::SocketSet sockset;

    while (1) {
        (sockset += *ssocket1) += *ssocket2;
        std::list<Net::Socket*> ready = sockset.select();

        for (std::list<Net::Socket*>::iterator cit = ready.begin(); cit != ready.end(); ++cit) {
                Net::ClientSocket* csock = dynamic_cast<Net::ClientSocket*>(*cit);

                // if this is a client socket, we need to ready the data from it to build the request
                if (csock) {

                    data[csock] += csock->recv();
                    // found request body delimiter
                    // TODO: implement request body support
                    if (data[csock].find(HTTP::BODY_DELIMITER) != std::string::npos) {
                        HTTP::Request req(data[csock]);
                        HTTP::Response res(req);

                        res.sendFile("./asset/www/index.html");
                        csock->send(res.str());

                        // socket cleanup

                        data.erase(csock);
                        sockset -= *csock;
                        csock->close();
                        delete csock;
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