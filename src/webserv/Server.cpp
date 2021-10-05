/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aldubar <aldubar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/10/05 11:04:48 by aldubar           #+#    #+#             */
/*   Updated: 2021/10/05 14:48:50 by aldubar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <sys/socket.h>
#include <cstdlib>
#include <iostream>
#include <unistd.h>

Server::Server( void ): _port(80), _maxConnexion(10) {

	init();
}

Server::Server( int port ): _port(port), _maxConnexion(10) {

	init();
}

Server::Server( Server const & src ) { *this = src; }

Server::~Server( void ) {}

Server &	Server::operator=( Server const & rhs ) {

	if (this != &rhs) {
		
		this->_port = rhs._port;
		this->_socketFd = rhs._socketFd;
	}

	return *this;
}

void		Server::init( void ) {

	_socketFd = socket(AF_INET, SOCK_STREAM, 0);

	if (_socketFd == -1) {

		std::cout << "Failed to create socket. errno: " << errno << " " << strerror(errno) << std::endl;
    	exit(EXIT_FAILURE);
	}

	_sockaddr.sin_family = AF_INET;
	_sockaddr.sin_addr.s_addr = INADDR_ANY;
	_sockaddr.sin_port = htons(_port);

	if (bind(_socketFd, (struct sockaddr *)&_sockaddr, sizeof(_sockaddr)) < 0) {

		std::cout << "Failed to bind to port " << _port << ": " << errno << " " << strerror(errno) << std::endl;
    	exit(EXIT_FAILURE);
	}

	if (listen(_socketFd, _maxConnexion) < 0) {

		std::cout << "Failed to listen on socket. errno: " << errno << " " <<  strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
}

void		Server::start( void ) {

	int		addrlen = sizeof(_sockaddr);

	while (1) {

		int		connection = accept(_socketFd, (struct sockaddr*)&_sockaddr, (socklen_t*)&addrlen);

		if (connection < 0) {

			std::cout << "Failed to grab connection. errno: " << errno << std::endl;
			exit(EXIT_FAILURE);
		}

		char	buffer[1024];
		bzero(buffer, 1024);

		int		bytesread = read(connection, buffer, 1024);

		if (!bytesread)
			std::cout << "nothing received..." << std::endl;
		else
			std::cout << buffer << std::endl << "---------------------" << std::endl;

		std::string		response = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 22\n\nWebserv is working!!!!";
		write(connection, response.c_str(), response.length());
//		send(connection, response.c_str(), response.size(), 0);

		close(connection);
	}

}

void		Server::stop( void ) {

	close(_socketFd);
}
