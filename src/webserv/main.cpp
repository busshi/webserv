#include "Server.hpp"
#include "webserv/config-parser/ConfigParser.hpp"
#include <iostream>

int main(int ac, char** av) {

    /*
    if (ac != 2) {

            //std::cout << "Usage: ./webserv [path to config file]" <<
    std::endl; std::cout << "Usage: ./webserv [port to bind]" << std::endl;
            return 0;
    }
        */

    Server server;

    try {
        (void)av;
        (void)ac;

        ConfigParser cfgp;

        WebservConfig* config = cfgp.loadConfig("./asset/config/example1.conf");

        server.init(av[1]);
        server.start();
        server.stop();

        delete config;
    } catch (Lexer::LexerException& e) {
        e.printFormatted(std::cerr) << "\n";
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
