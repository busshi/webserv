#include "Server.hpp"
#include <iostream>
#include <cstdlib>

int		main( int ac, char **av ) {

	if (ac != 2) {

		//std::cout << "Usage: ./webserv [path to config file]" << std::endl;
		std::cout << "Usage: ./webserv [port to bind]" << std::endl;
		return 0;
	}

//	char const *	configFile = av[1];
	
	int		port = atoi(av[1]);
	Server	server(port);

	server.start();
	server.stop();

	return 0;
}
