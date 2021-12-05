#pragma once
#include "cgi/cgi.hpp"
#include "http/message.hpp"
#include "webserv/config-parser/ConfigParser.hpp"
#include <map>
#include <netinet/in.h>

#define BUFSIZE 1024

void
initHosts(ConfigItem* global);

void
destroyHosts(void);

void
lifecycle(const HttpParser::Config& parserConf);

void
onHeader(const std::string& method,
         const std::string& loc,
         const std::string& protocol,
         uintptr_t requestLoc);

void
onHeaderField(const std::string& name,
              const std::string& value,
              uintptr_t requestLoc);

void
onHeaderParsed(uintptr_t requestLoc);

void
onBodyFragment(const std::string& fragment, uintptr_t requestLoc);

void
onBodyChunk(const std::string& chunk, uintptr_t requestLoc);

void
onBodyUnchunked(uintptr_t requestLoc);

void
onBodyParsed(uintptr_t requestLoc);

void
createResponse(HTTP::Request& req, HTTP::Response& res, ConfigItem* server);

ConfigItem*
selectServer(std::vector<ConfigItem*>& candidates, const std::string& host);

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
extern fd_set select_rset;
extern fd_set select_wset;
extern bool isWebservAlive;
