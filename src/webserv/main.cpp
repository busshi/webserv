#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <iostream>
#include <unistd.h>

int		server( int port ) {
	
	int		server_fd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (server_fd == -1) {
    
		std::cout << "Failed to create socket. errno: " << errno << " " << strerror(errno) << std::endl;
    	exit(EXIT_FAILURE);
	}
	
	sockaddr_in sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	sockaddr.sin_port = htons(port);
	
	if (bind(server_fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0) {
    
		std::cout << "Failed to bind to port " << port << ": " << errno << " " << strerror(errno) << std::endl;
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

int		main( int ac, char **av ) {

	if (ac != 2) {

		//std::cout << "Usage: ./webserv [path to config file]" << std::endl;
		std::cout << "Usage: ./webserv [port to bind]" << std::endl;
		return 0;
	}

//	char const *	configFile = av[1];

	server(std::stoi(av[1]));

	return 0;
}
