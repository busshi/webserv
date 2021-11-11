#include "Server.hpp"
#include <unistd.h>
#include <sys/select.h>
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
		this->_sockets = rhs._sockets;
        this->_connexion = rhs._connexion;
    }

    return *this;
}

void Server::init( ConfigItem * global )
{
	std::vector<ConfigItem*> serverBlocks = global->findBlocks("server");

	for (std::vector<ConfigItem*>::const_iterator ite = serverBlocks.begin();
	    ite != serverBlocks.end(); ++ite) {
			
		std::vector<ConfigItem*>	listens = (*ite)->findBlocks("listen");

		for (size_t i = 0; i != listens.size(); i++) {

			ListenData	data = parseListen(listens[i]->getValue());
			unsigned short	port = data.port;

			if (port) {

				_sockets[port].socket = -1;
				_sockets[port].maxConnexion = 10;
			}

			ConfigItem *	path = (*ite)->findNearestAtom("root");

			if (path)
				_sockets[port].root = path->getValue();
			
			else {
				
				_sockets[port].root = "none";
				std::cout << RED << "Error: No default path provided!" << CLR << std::endl;
			}
		}
	}
}

int		Server::_createSocket( void ) {

    int	socketFd = socket(AF_INET, SOCK_STREAM, 0);

    if (socketFd == -1) {
        std::cout << RED << "Failed to create socket. errno: " << errno << " "
                  << strerror(errno) << CLR << std::endl;
        exit(EXIT_FAILURE);
    }
	return socketFd;
}

sockaddr_in	Server::_bindPort( int socketFd, unsigned short port ) {

	sockaddr_in	sockaddr;

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(port);

    if (bind(socketFd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
        std::cout << RED << "Failed to bind to port " << port << ": [" << errno << "] "
                  << strerror(errno) << CLR << std::endl;
        exit(EXIT_FAILURE);
    }
	else
		std::cout << GREEN << "webserv listening on port " << BOLD << port << CLR << std::endl;
	
	return sockaddr;
}

void		Server::_listenSocket( int socketFd, int maxConnexion ) {

    if (listen(socketFd, maxConnexion) < 0) {
        std::cout << RED << "Failed to listen on socket. errno: [" << errno << "] "
                  << strerror(errno) << CLR << std::endl;
        exit(EXIT_FAILURE);
	}
}

int		Server::_accept( int socketFd, sockaddr_in sockaddr, int addrlen ) {

        int connexion =
          accept(socketFd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);

        if (connexion < 0) {
            std::cout << RED << "Failed to grab connection. errno: [" << errno << "] "
                      << strerror(errno) << CLR << std::endl;
            exit(EXIT_FAILURE);
        }
		
		return connexion;
}

void
Server::start(void)
{
	std::map<unsigned short, Socket>::iterator	it, ite = _sockets.end();

	for (it = _sockets.begin(); it != ite; it++) {
		
		_sockets[it->first].socket = _createSocket();
		_sockets[it->first].sockaddr = _bindPort(_sockets[it->first].socket, it->first);
		_listenSocket(_sockets[it->first].socket, _sockets[it->first].maxConnexion);
		_sockets[it->first].addrlen = sizeof(_sockets[it->first].sockaddr);
	}

    while (1) {

		fd_set			readfds;
//		fd_set			writefds;

		struct timeval	timeout;

		int				ret = 0;
		int				i = 0;
		std::string		array[] = {".    ", "..   ", "...  ", ".... ", "....."};

		while (!ret) {

			int	nfds = 1024;

			timeout.tv_sec = 1;
			timeout.tv_usec = 0;
			
			FD_ZERO(&readfds);

			std::map<unsigned short, Socket>::iterator it, ite = _sockets.end();
			for (it = _sockets.begin(); it != ite; it++)
				FD_SET(_sockets[it->first].socket, &readfds);

			std::cout << "\rWaiting for connection" << array[i++] << std::flush;

			if (i == 5)
				i = 0;

			ret = select(nfds, &readfds, NULL, NULL, &timeout);
		}

		if (ret > 0) {	

			std::cout << ORANGE << "\n\nConnexion received.\n" << CLR << std::endl;

			std::map<unsigned short, Socket>::iterator it, ite = _sockets.end();
			for (it = _sockets.begin(); it != ite; it++) {
				if (FD_ISSET(_sockets[it->first].socket, &readfds))	{

					_connexion = _accept(_sockets[it->first].socket, _sockets[it->first].sockaddr, _sockets[it->first].addrlen);
        	
					char buffer[1024];
        			bzero(buffer, 1024);

        			int bytesread = read(_connexion, &buffer, 1024);

        			if (!bytesread)
        	    		std::cout << "nothing received..." << std::endl;
					else
        	    		std::cout << PURPLE << "----- Received Header -----\n" << CLR << buffer << std::endl;

					Header	header;

        			header.parseHeader(buffer, _sockets[it->first].root);
        			header.createResponse();
        			sendResponse(header);
        			close(_connexion);
				}
			}
		}
    }
}

void
Server::sendResponse( Header header )
{
	std::string	response = header.getResponse();

	if (response.size() > 512)
		std::cout << PURPLE << "----- Response Header -----" << CLR << std::endl << response.substr(0, 512) << "\n\n[ ...SNIP... ]" << std::endl;
	else
		std::cout << PURPLE << "----- Response Header -----" << CLR << std::endl << response << std::endl;

    //write(_connexion, response.c_str(), response.length());
    send(_connexion, response.c_str(), response.size(), 0);
}

void
Server::stop(void)
{
	std::map<unsigned short, Socket>::iterator it, ite = _sockets.end();

	for (it = _sockets.begin(); it != ite; it++) {
    	
		if (_sockets[it->first].socket > 0)
			close(_sockets[it->first].socket);
	}
}
