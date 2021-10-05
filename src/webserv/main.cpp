#include "Server.hpp"
#include <iostream>

int		main( int ac, char **av ) {

	if (ac != 2) {

		//std::cout << "Usage: ./webserv [path to config file]" << std::endl;
		std::cout << "Usage: ./webserv [port to bind]" << std::endl;
		return 0;
	}

//	char const *	configFile = av[1];
	
	Server	server(std::stoi(av[1]));

	server.start();
	server.stop();

	return 0;
}
