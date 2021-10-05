/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aldubar <aldubar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/10/05 11:04:48 by aldubar           #+#    #+#             */
/*   Updated: 2021/10/05 12:41:02 by aldubar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <iostream>
#include <unistd.h>

Server::Server( void ): _port(80) {}

Server::Server( int port ): _port(port) {}

Server::Server( Server const & src ) { *this = src; }

Server::~Server( void ) {}

Server &	Server::operator=( Server const & rhs ) {

	if (this != &rhs)
		this->_port = rhs._port;

	return *this;
}

int			Server::start( void ) {

	int		server_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (server_fd == -1) {

		std::cout << "Failed to create socket. errno: " << errno << " " << strerror(errno) << std::endl;
    	exit(EXIT_FAILURE);
	}

	sockaddr_in sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	sockaddr.sin_port = htons(_port);

	if (bind(server_fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0) {

		std::cout << "Failed to bind to port " << _port << ": " << errno << " " << strerror(errno) << std::endl;
    	exit(EXIT_FAILURE);
	}

	if (listen(server_fd, 10) < 0) {

		std::cout << "Failed to listen on socket. errno: " << errno << " " <<  strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}

	int		addrlen = sizeof(sockaddr);

	while (1) {

		int		connection = accept(server_fd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);

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

	close(server_fd);

	return 0;
}
