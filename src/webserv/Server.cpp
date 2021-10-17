#include "Server.hpp"
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

Server::Server(void) {}

Server::Server(Server const& src)
{
    *this = src;
}

Server::~Server(void) {}

Server&
Server::operator=(Server const& rhs)
{
    if (this != &rhs) {
        this->_port = rhs._port;
        this->_socketFd = rhs._socketFd;
        this->_maxConnexion = rhs._maxConnexion;
        this->_connexion = rhs._connexion;
    }

    return *this;
}

void Server::init( ConfigItem * global )
{
	std::vector<ConfigItem*> serverBlocks = global->findBlocks("server");

	for (std::vector<ConfigItem*>::const_iterator ite = serverBlocks.begin();
	    ite != serverBlocks.end(); ++ite) {
			
			ConfigItem *	port = (*ite)->findAtomInBlock("listen");
		
			if (port) {

				_port = atoi(port->getValue().c_str());
				std::cout << "webserv listening on port " << _port << std::endl;
			}

			ConfigItem *	path = (*ite)->findAtomInBlock("root");
			if (path)
				_rootPath = path->getValue();
			else
				std::cout << "Error: No default path provided!" << std::endl;
	}

    _maxConnexion = 10;
}

void
Server::start(void)
{
    _socketFd = socket(AF_INET, SOCK_STREAM, 0);

    if (_socketFd == -1) {
        std::cout << "Failed to create socket. errno: " << errno << " "
                  << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    sockaddr_in sockaddr;

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(_port);

    if (bind(_socketFd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
        std::cout << "Failed to bind to port " << _port << ": " << errno << " "
                  << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    if (listen(_socketFd, _maxConnexion) < 0) {
        std::cout << "Failed to listen on socket. errno: " << errno << " "
                  << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    int addrlen = sizeof(sockaddr);

    while (1) {
        _connexion =
          accept(_socketFd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);

        if (_connexion < 0) {
            std::cout << "Failed to grab connection. errno: " << errno
                      << std::endl;
            exit(EXIT_FAILURE);
        }

        char buffer[1024];
        bzero(buffer, 1024);

        int bytesread = read(_connexion, &buffer, 1024);

        if (!bytesread)
            std::cout << "nothing received..." << std::endl;
		else
            std::cout << "----- Received Header -----\n" << buffer << std::endl;

		Header	header;

        header.parseHeader(buffer, _rootPath);
        header.createResponse();
        sendResponse(header);
        close(_connexion);
    }
}

void
Server::sendResponse( Header header )
{
	std::string	response = header.getResponse();

	if (response.size() > 512)
		std::cout << "----- Response Header -----" << std::endl << response.substr(0, 512) << "\n\n[ ...SNIP... ]" << std::endl;
	else
		std::cout << "----- Response Header -----" << std::endl << response << std::endl;

    //write(_connexion, response.c_str(), response.length());
    send(_connexion, response.c_str(), response.size(), 0);
}

void
Server::stop(void)
{
    close(_socketFd);
}
