#include "Server.hpp"
#include "Constants.hpp"
#include <iostream>
#include <signal.h>
#include <cstdlib>
#include "logger/Logger.hpp"

Logger glogger("logs", "log.txt");

bool isWebservAlive = true;

static void handleSIGINT(int)
{
    std::cout << " Bye bye!\n";
    isWebservAlive = false;
}

int
main(int ac, char** av)
{
    if (ac != 2) {
        std::cout << "Usage: ./webserv [path to config file]" << std::endl;
        return EXIT_FAILURE;
    }

    signal(SIGINT, handleSIGINT);

    try {
        ConfigParser cfgp;

        ConfigItem* config = cfgp.loadConfig(av[1]);
        ConfigItem* log_level = config->findNearest("log_level");
        
        if (log_level) {
            glogger.setWebservLogLevel(Logger::parseLogLevel(log_level->getValue()));
        }

        Server server(config);

        server.start();
    } catch (Lexer::LexerException& e) {
        e.printFormatted(std::cerr) << "\n";
    } catch (ConfigParser::ParserException& e) {
        e.printFormatted(std::cerr) << "\n";
    } catch (std::exception& e) {
        std::cout << RED << e.what() << CLR << std::endl;
    }

    return 0;
}
