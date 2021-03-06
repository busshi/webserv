#pragma once
#include "Buffer.hpp"
#include "FileUploader.hpp"
#include "cgi/cgi.hpp"
#include "config/ConfigParser.hpp"
#include "http/Exception.hpp"
#include "http/message.hpp"
#include "utils/Logger.hpp"
#include <map>
#include <netinet/in.h>
#include <stdint.h>

#define CLOSE_FD(fd)                                                           \
    if (fd != -1)                                                              \
        close(fd);                                                             \
    fd = -1;

/* This is the size of the buffer used to recv data from the HTTP client.
 * The greater this size is, the faster the server will (probably) be.
 * In other words, setting it to 1 is funny and cool for debuging purpose but
 * not really optimal :')
 */

#define BUFSIZE 0xFFFF

#define DFLT_MAX_UPLOAD_FILE_SIZE "1MB"

#define DFLT_MAX_BODY_SIZE "100MB"

void
handleHttpException(HTTP::Exception& e);

void
initHosts(ConfigItem* global);

void
destroyHosts(void);

void
lifecycle(const HTTP::MessageParser::Config& parserConf,
          unsigned long long requestTimeout);

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
onBodyFragment(const Buffer<>& fragment, uintptr_t requestLoc);

void
onBodyChunk(const Buffer<>& chunk, uintptr_t requestLoc);

void
onBodyUnchunked(uintptr_t requestLoc);

void
onBodyParsed(uintptr_t requestLoc);

void
createResponse(HTTP::Request& req, HTTP::Response& res, ConfigItem* server);

void
processRequest(HTTP::Request* req);

struct Host
{
    int ssockFd;
    sockaddr_in addr;
    std::vector<ConfigItem*> candidates; /* server blocks that are related */
};

/* webserv global variables */

extern std::map<int, HTTP::Request*> requests;
extern std::map<int, CGI*> cgis;
extern std::map<int, FileUploader*> uploaders;
extern std::map<uint16_t, Host> hosts;
extern fd_set select_rset;
extern fd_set select_wset;
extern bool isWebservAlive;
extern Logger glogger;
