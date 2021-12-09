#include "Constants.hpp"
#include "HttpParser.hpp"
#include "cgi/cgi.hpp"
#include "config/ConfigParser.hpp"
#include "core.hpp"
#include "http/message.hpp"
#include "utils/BinBuffer.hpp"
#include "utils/Logger.hpp"
#include <map>
#include <unistd.h>

using std::map;

std::map<int, HTTP::Request*> requests;
std::map<int, CommonGatewayInterface*> cgis;
std::map<uint16_t, Host> hosts;
fd_set select_rset, select_wset;

bool isWebservAlive = true;

#ifdef LOGGER
Logger glogger;
#endif

int
main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "Usage: ./webserv <path/to/config/file>\n";
        return 1;
    }

#ifdef LOGGER
    glogger << Logger::getTimestamp() << " Webserv started\n";
#endif

    ConfigParser cfgp;
    ConfigItem* global;

    try {
        global = cfgp.loadConfig(argv[1]);
    } catch (Lexer::LexerException& e) {
        e.printFormatted(std::cerr) << "\n";
        return 1;
    } catch (ConfigParser::ParserException& e) {
        e.printFormatted(std::cerr) << "\n";
        return 1;
    }

#ifdef LOGGER
    glogger << Logger::getTimestamp() << " Configuration loaded from file "
            << argv[1] << "\n";
#endif

#ifdef LOGGER
    ConfigItem* logLevel = global->findNearest("logLevel");

    if (logLevel) {
        glogger << "Log level set to \"" << logLevel->getValue() << "\"\n";
        glogger.setWebservLogLevel(Logger::parseLogLevel(logLevel->getValue()));
    }
#endif

    FD_ZERO(&select_rset);
    FD_ZERO(&select_wset);

    initHosts(global);

    HttpParser::Config parserConf;

    memset(&parserConf, 0, sizeof(parserConf));

    parserConf.onHeader = onHeader;
    parserConf.onHeaderField = onHeaderField;
    parserConf.onBodyChunk = onBodyChunk;
    parserConf.onBodyFragment = onBodyFragment;
    parserConf.onHeaderParsed = onHeaderParsed;
    parserConf.onBodyUnchunked = onBodyUnchunked;
    parserConf.onBodyParsed = onBodyParsed;

    std::cout << "webserv - a tiny HTTP server implementation\nBy abrabant and "
                 "aldubar\n\nConfiguration loaded from: "
              << argv[1] << "\n";

#ifdef LOGGER
    std::cout << "This version of webserv has been compiled with event logging "
                 "support\nLog file is located at "
              << glogger.getLogPath() << "\nLog level is "
              << (logLevel ? logLevel->getValue() : "INFO") << "\n";
#endif

    std::cout << "\n";

    lifecycle(parserConf);

    destroyHosts();

    for (map<int, CommonGatewayInterface*>::const_iterator cit = cgis.begin();
         cit != cgis.end();
         ++cit) {
        CommonGatewayInterface* cgi = cit->second;

        delete cgi;
    }

    for (map<int, HTTP::Request*>::const_iterator cit = requests.begin();
         cit != requests.end();
         ++cit) {
        delete cit->second;
        close(cit->first);
    }

    delete global;

    return 0;
}
