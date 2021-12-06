#include "HttpParser.hpp"
#include "cgi/cgi.hpp"
#include "config/ConfigParser.hpp"
#include "core.hpp"
#include "http/message.hpp"
#include "utils/Logger.hpp"
#include <map>

std::map<int, HTTP::Request*> requests;
std::map<int, CommonGatewayInterface*> cgis;
std::map<uint16_t, Host> hosts;
fd_set select_rset, select_wset;

bool isWebservAlive = true;
Logger glogger;

int
main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "Usage: ./webserv <path/to/config/file>\n";
        return 1;
    }

    glogger << Logger::getTimestamp() << " Webserv started\n";

    ConfigParser cfgp;
    ConfigItem* global = cfgp.loadConfig(argv[1]);

    glogger << Logger::getTimestamp() << " Configuration loaded from file "
            << argv[1] << "\n";

    ConfigItem* logLevel = global->findNearest("logLevel");

    if (logLevel) {
        glogger << "Log level set to \"" << logLevel->getValue() << "\"\n";
        glogger.setWebservLogLevel(Logger::parseLogLevel(logLevel->getValue()));
    }

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

    lifecycle(parserConf);

    destroyHosts();

    return 0;
}
