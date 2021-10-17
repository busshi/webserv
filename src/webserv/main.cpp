#include "Server.hpp"
#include <iostream>

int
main(int ac, char** av)
{
    if (ac != 2) {
        std::cout << "Usage: ./webserv [path to config file]" << std::endl;
        //std::cout << "Usage: ./webserv [port to bind]" << std::endl;
        return 0;
    }

    Server server;

    try {
        ConfigParser cfgp;

        //ConfigItem* config = cfgp.loadConfig("./asset/config/example1.conf");
        ConfigItem* config = cfgp.loadConfig(av[1]);

        // cfgp.printConfig(std::cout, config);

        std::vector<ConfigItem*> civ = config->findBlocks("server");

//        for (size_t i = 0; i != civ.size(); ++i) {
  //          std::vector<ConfigItem*> civ2 = civ[i]->findBlocks("location");

    //        for (size_t j = 0; j != civ2.size(); ++j) {
      //          std::cout << *(civ2[j]) << "\n";
        //    }
       // }

        

        server.init(config);
        server.start();
        server.stop();

        

        delete config;
    } catch (Lexer::LexerException& e) {
        e.printFormatted(std::cerr) << "\n";
    } catch (ConfigParser::ParserException& e) {
        e.printFormatted(std::cerr) << "\n";
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
