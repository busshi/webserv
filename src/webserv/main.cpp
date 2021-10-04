#include <iostream>

int		main( void ) {

	if (ac != 2) {

		std::cout << "Usage: ./webserv [path to config file]" << std::endl;
	}

	char const *	configFile = av[1];

	return 0;
}
