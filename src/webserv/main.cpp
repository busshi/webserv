#include <iostream>

#include "Server.hpp"

int
main(int ac, char** av)
{
    if (ac != 2) {
        // std::cout << "Usage: ./webserv [path to config file]" << std::endl;
        std::cout << "Usage: ./webserv [port to bind]" << std::endl;
        return 0;
    }

    Server server;

    try {
        server.init(av[1]);
        server.start();
        server.stop();
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
