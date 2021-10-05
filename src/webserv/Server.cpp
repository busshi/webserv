/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aldubar <aldubar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/10/05 11:04:48 by aldubar           #+#    #+#             */
/*   Updated: 2021/10/05 15:17:19 by aldubar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <unistd.h>

Server::Server( void ): _port(80), _maxConnexion(10) {}

Server::Server( int port ): _port(port), _maxConnexion(10) {}

Server::Server( Server const & src ) { *this = src; }

Server::~Server( void ) {}

Server &	Server::operator=( Server const & rhs ) {

	if (this != &rhs) {
		
		this->_port = rhs._port;
		this->_socketFd = rhs._socketFd;
		this->_maxConnexion = rhs._maxConnexion;
		this->_connexion = rhs._connexion;
	}

	return *this;
}

void		Server::start( void ) {

	_socketFd = socket(AF_INET, SOCK_STREAM, 0);

	if (_socketFd == -1) {

		std::cout << "Failed to create socket. errno: " << errno << " " << strerror(errno) << std::endl;
    	exit(EXIT_FAILURE);
	}

	sockaddr_in		sockaddr;

	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	sockaddr.sin_port = htons(_port);

	if (bind(_socketFd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0) {

		std::cout << "Failed to bind to port " << _port << ": " << errno << " " << strerror(errno) << std::endl;
    	exit(EXIT_FAILURE);
	}

	if (listen(_socketFd, _maxConnexion) < 0) {

		std::cout << "Failed to listen on socket. errno: " << errno << " " <<  strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}

	int		addrlen = sizeof(sockaddr);

	while (1) {

		_connexion = accept(_socketFd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);

		if (_connexion < 0) {

			std::cout << "Failed to grab connection. errno: " << errno << std::endl;
			exit(EXIT_FAILURE);
		}

		char	buffer[1024];
		bzero(buffer, 1024);

		int		bytesread = read(_connexion, buffer, 1024);

		if (!bytesread)
			std::cout << "nothing received..." << std::endl;
		else
			std::cout << buffer << std::endl << "---------------------" << std::endl;

		parseHeader();
		sendResponse();
		close(_connexion);
	}

}

void		Server::parseHeader( void ) {

}

void		Server::sendResponse( void ) {

		std::string		response = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 22\n\nWebserv is working!!!!";

		write(_connexion, response.c_str(), response.length());
//		send(connexion, response.c_str(), response.size(), 0);
}

void		Server::stop( void ) {

	close(_socketFd);
}
