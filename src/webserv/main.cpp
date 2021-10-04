#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <iostream>
#include <unistd.h>

int		server( int port ) {
	
	int		sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sockfd == -1) {
    
		std::cout << "Failed to create socket. errno: " << errno << " " << strerror(errno) << std::endl;
    	exit(EXIT_FAILURE);
	}
	
	sockaddr_in sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	sockaddr.sin_port = htons(port);
	
	if (bind(sockfd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0) {
    
		std::cout << "Failed to bind to port " << port << ": " << errno << " " << strerror(errno) << std::endl;
    	exit(EXIT_FAILURE);
	}
	
	if (listen(sockfd, 10) < 0) {
    
		std::cout << "Failed to listen on socket. errno: " << errno << " " <<  strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
	
	auto	addrlen = sizeof(sockaddr);
	int		connection = accept(sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
	
	if (connection < 0) {
		
		std::cout << "Failed to grab connection. errno: " << errno << std::endl;
		exit(EXIT_FAILURE);
	}
	
	char	buffer[100];
	auto	bytesRead = read(connection, buffer, 100);
	
	std::cout << "Client said: " << buffer;
	
	std::string response = "Copy that\n";
	send(connection, response.c_str(), response.size(), 0);
	
	close(connection);
	close(sockfd);

	return 0;
}

int		main( int ac, char **av ) {

	if (ac != 2) {

		//std::cout << "Usage: ./webserv [path to config file]" << std::endl;
		std::cout << "Usage: ./webserv [port to listen]" << std::endl;
		return 0;
	}

//	char const *	configFile = av[1];

	server(std::stoi(av[1]));

	return 0;
}
