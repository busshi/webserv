#include "Server.hpp"
#include <unistd.h>
#include <sys/select.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

#include <stdio.h>
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
			
//			std::vector<ConfigItem*>	listens = (*ite)->findBlocks("listen");

//			for (size_t i = 0; i != listens.size(); i++) {

//				ListenData	data = parseListen(listens[i]->getValue());
//				_port = data.port;
//				std::cout << _port << std::endl;
//			}

			ConfigItem *	port = (*ite)->findAtomInBlock("listen");
		
			if (port) {

				_port = atoi(port->getValue().c_str());
//				std::cout << "webserv listening on port " << _port << std::endl;
			}

			ConfigItem *	path = (*ite)->findAtomInBlock("root");
			if (path)
				_rootPath = path->getValue();
			else
				std::cout << "Error: No default path provided!" << std::endl;
	}

    _maxConnexion = 10;
}

int		Server::_createSocket( void ) {

    int	socketFd = socket(AF_INET, SOCK_STREAM, 0);

    if (socketFd == -1) {
        std::cout << "Failed to create socket. errno: " << errno << " "
                  << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
	return socketFd;
}

sockaddr_in	Server::_bindPort( int socketFd, int port ) {

	sockaddr_in	sockaddr;

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(port);

    if (bind(socketFd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
        std::cout << "Failed to bind to port " << port << ": " << errno << " "
                  << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
	else
		std::cout << "webserv listening on port " << port << std::endl;
	
	return sockaddr;
}

void		Server::_listenSocket( int socketFd, int maxConnexion ) {

    if (listen(socketFd, maxConnexion) < 0) {
        std::cout << "Failed to listen on socket. errno: " << errno << " "
                  << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
	}
}

int		Server::_accept( int socketFd, sockaddr_in sockaddr, int addrlen ) {

        int connexion =
          accept(socketFd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);

        if (connexion < 0) {
            std::cout << "Failed to grab connection. errno: " << errno
                      << std::endl;
            exit(EXIT_FAILURE);
        }
		
		return connexion;
}

void
Server::start(void)
{
	_socketFd = _createSocket();
    sockaddr_in sockaddr = _bindPort(_socketFd, _port);
	_listenSocket(_socketFd, _maxConnexion);

	int	socketFd2 = _createSocket();
	sockaddr_in	sockaddr2 = _bindPort(socketFd2, 9090);
	_listenSocket(socketFd2, _maxConnexion);

   	int addrlen = sizeof(sockaddr);
    int addrlen2 = sizeof(sockaddr2);

    while (1) {

		fd_set			readfds;
//		fd_set			writefds;

		struct timeval	timeout;

		int				ret = 0;
		int				i = 0;

		while (!ret) {

			int	nfds = _maxConnexion;

			timeout.tv_sec = 1;
			timeout.tv_usec = 0;
			
			FD_ZERO(&readfds);
			FD_SET(_socketFd, &readfds);
			FD_SET(socketFd2, &readfds);

			std::cout << "\rWaiting for connection..." << i++ << std::flush;

			ret = select(nfds, &readfds, NULL, NULL, &timeout);
		}

		if (ret > 0) {	

			std::cout << std::endl << "Connexion received." << std::endl;

			if (FD_ISSET(_socketFd, &readfds))				
				_connexion = _accept(_socketFd, sockaddr, addrlen);

			if (FD_ISSET(socketFd2, &readfds))
				_connexion = _accept(socketFd2, sockaddr2, addrlen2);
				

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
