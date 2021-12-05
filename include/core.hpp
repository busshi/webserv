#pragma once
#include "cgi/cgi.hpp"
#include "http/message.hpp"
#include "webserv/config-parser/ConfigParser.hpp"
#include <map>
#include <netinet/in.h>

void
initHosts(ConfigItem* global, fd_set& rset);

void
destroyHosts(void);

void
lifecycle(const HttpParser::Config& parserConf, fd_set& rset, fd_set& wset);

struct Host
{
    int ssockFd;
    sockaddr_in addr;
    std::vector<ConfigItem*> candidates; /* server blocks that are related */
};

/* webserv global variables */

extern std::map<int, HTTP::Request*> requests;
extern std::map<int, CommonGatewayInterface*> cgis;
extern std::map<uint16_t, Host> hosts;
extern bool isWebservAlive;
