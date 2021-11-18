#include "Server.hpp"
#include "logger/Logger.hpp"
#include "utils/string.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>
#include <stdexcept>

Server::Server( ConfigItem * global ) {

	std::vector<ConfigItem*> serverBlocks = global->findBlocks("server");

	_config = global;

	for (std::vector<ConfigItem*>::const_iterator it = serverBlocks.begin();
	    it != serverBlocks.end(); ++it) {
			
		std::vector<ConfigItem*>	listens = (*it)->findBlocks("listen");

		for (size_t i = 0; i != listens.size(); i++) {

			ListenData	data = parseListen(listens[i]->getValue());
			unsigned short	port = data.port;

			_sockets[port].socket = -1;
			_sockets[port].ipv4 = data.v4;
			_sockets[port].maxConnexion = 10;
			_sockets[port].item = *it;
		}
	}
}

Server::Server(Server const& src)
{
    *this = src;
}

Server::~Server(void) {

	std::map<unsigned short, Socket>::iterator it, ite = _sockets.end();

	for (it = _sockets.begin(); it != ite; it++) {
    	
		if (_sockets[it->first].socket != -1)
			close(_sockets[it->first].socket);
	}

	delete _config;
}

Server&
Server::operator=(Server const& rhs)
{
    if (this != &rhs) {
		this->_sockets = rhs._sockets;
        this->_connexion = rhs._connexion;
    }

    return *this;
}

int		Server::_createSocket( void ) {

    int	socketFd = socket(AF_INET, SOCK_STREAM, 0);

    if (socketFd == -1) {
        glogger << Logger::ERROR << Logger::getTimestamp() << RED << " Failed to create socket. errno: " << errno << " "
                  << strerror(errno) << CLR << "\n";
		throw std::runtime_error(std::string("Error creating socket"));
    }
	fcntl(socketFd, F_SETFL, O_NONBLOCK);

	return socketFd;
}

sockaddr_in	Server::_bindPort( int socketFd, unsigned short port, uint32_t ipv4 ) {

	sockaddr_in	sockaddr;

    sockaddr.sin_family = AF_INET;
   	sockaddr.sin_addr.s_addr = ipv4;
    sockaddr.sin_port = htons(port);

    if (bind(socketFd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
        glogger << Logger::ERROR << Logger::getTimestamp() << RED << " Failed to bind to port " << port << ": [" << errno << "] "
                  << strerror(errno) << CLR << "\n";
		throw std::runtime_error("webserv: fatal error: impssible to bind port");
    }
	else
		glogger << Logger::INFO << Logger::getTimestamp() << GREEN << " webserv listening on port " << BOLD << port << CLR << "\n";
	
	return sockaddr;
}

void		Server::_listenSocket( int socketFd, int maxConnexion ) {

    if (listen(socketFd, maxConnexion) < 0) {
        glogger << Logger::ERROR << Logger::getTimestamp() << RED << " Failed to listen on socket. errno: [" << errno << "] "
                  << strerror(errno) << CLR << "\n";
		throw std::runtime_error(std::string("webserv: fatal error: impossible to listen on socket"));
	}
}

int		Server::_accept( int socketFd, sockaddr_in sockaddr, int addrlen ) {

        int connexion =
          accept(socketFd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);

        if (connexion < 0) {
            glogger << Logger::ERROR << Logger::getTimestamp() << RED << " Failed to grab connection. errno: [" << errno << "] "
                      << strerror(errno) << CLR << "\n";
			throw std::runtime_error(std::string("webserv: error: grabing connection"));
        }
		
		return connexion;
}

void
Server::start(void)
{
	std::map<unsigned short, Socket>::iterator	it, ite = _sockets.end();

	for (it = _sockets.begin(); it != ite; it++) {
		
		_sockets[it->first].socket = _createSocket();
		_sockets[it->first].sockaddr = _bindPort(_sockets[it->first].socket, it->first, _sockets[it->first].ipv4);
		_listenSocket(_sockets[it->first].socket, _sockets[it->first].maxConnexion);
		_sockets[it->first].addrlen = sizeof(_sockets[it->first].sockaddr);
	}

	std::cout << "webserv is running\nHit Ctrl-C to exit." << std::endl;

    while (isWebservAlive) {

		fd_set			readfds;
		fd_set			writefds;

		struct timeval	timeout;

		int				ready = 0;

		int	nfds = 1024;

		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);

		std::map<unsigned short, Socket>::iterator it, ite = _sockets.end();
		for (it = _sockets.begin(); it != ite; it++){

			FD_SET(_sockets[it->first].socket, &readfds);
			FD_SET(_sockets[it->first].socket, &writefds);
		}
		ready = select(nfds, &readfds, &writefds, NULL, &timeout);

		if (ready > 0) {	

			std::map<unsigned short, Socket>::iterator it, ite = _sockets.end();
			for (it = _sockets.begin(); it != ite; it++) {

				if (FD_ISSET(_sockets[it->first].socket, &readfds))	{
			
					glogger << Logger::INFO << "\n\n" << Logger::getTimestamp() << GREEN << " Connexion received\n\n" << CLR;

					_connexion = _accept(_sockets[it->first].socket, _sockets[it->first].sockaddr, _sockets[it->first].addrlen);
        	

					char buffer[1024];
        			bzero(buffer, 1024);

        			int bytesread = read(_connexion, &buffer, 1024);

        			if (!bytesread)
        	    		glogger << Logger::WARNING << Logger::getTimestamp() << ORANGE << " Nothing received...\n" << CLR;
					else {

        	    		glogger << Logger::DEBUG << Logger::getTimestamp() << PURPLE << " Received Header:\n\n" << CLR << buffer << "\n";
					
//						Header	header;

//	        			header.parseHeader(buffer);
						Header	header(buffer);
						header.createResponse(_sockets[it->first].item);
	        			sendResponse(header);
					}
					FD_CLR(_sockets[it->first].socket, &readfds);
					FD_CLR(_sockets[it->first].socket, &writefds);
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
		glogger << Logger::DEBUG << "\n" << Logger::getTimestamp() << PURPLE << " Response Header:\n\n" << CLR << response.substr(0, 512) << "\n\n[ ...SNIP... ]\n";
	else
		glogger << Logger::DEBUG << "\n" << Logger::getTimestamp() << PURPLE << " Response Header:\n\n" << CLR << response << "\n";

    //write(_connexion, response.c_str(), response.length());
    send(_connexion, response.c_str(), response.size(), 0);
}
