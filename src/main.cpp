#include "HttpParser.hpp"
#include "cgi/cgi.hpp"
#include "core.hpp"
#include "http/message.hpp"
#include "logger/Logger.hpp"
#include "webserv/config-parser/ConfigParser.hpp"
#include <map>

std::map<int, HTTP::Request*> requests;
std::map<int, CommonGatewayInterface*> cgis;
std::map<uint16_t, Host> hosts;
bool isWebservAlive = true;
Logger glogger;

void
logHeader(const std::string& name, const std::string& value, uintptr_t request)
{
    (void)request;

    std::cout << name << ": " << value << std::endl;
}

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

    fd_set rset, wset;

    FD_ZERO(&rset);
    FD_ZERO(&wset);

    initHosts(global, rset);

    HttpParser::Config parserConf;

    memset(&parserConf, 0, sizeof(parserConf));
    parserConf.onHeaderField = logHeader;

    lifecycle(parserConf, rset, wset);

    destroyHosts();

    return 0;
}
