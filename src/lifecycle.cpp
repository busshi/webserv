#include "Constants.hpp"
#include "core.hpp"
#include "event.hpp"
#include "http/Exception.hpp"
#include "http/MessageParser.hpp"
#include "http/status.hpp"
#include "utils/Logger.hpp"
#include "utils/string.hpp"
#include <cstring>
#include <fcntl.h>
#include <iomanip>
#include <signal.h>
#include <sys/errno.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

using HTTP::MessageParser;

char eventBuf[BUFSIZE];

static void
handleSigint(int)
{
    isWebservAlive = false;
    std::cout << "\n" BOLD << "Stopping webserv..." << CLR << std::endl;
}

static void
handleSigpipe(int)
{
    std::cout << "Received SIGPIPE" << std::endl;
}

void
lifecycle(const MessageParser::Config& parserConf,
          unsigned long long requestTimeout)
{
    signal(SIGINT, &handleSigint);
    signal(SIGPIPE, &handleSigpipe);

    while (isWebservAlive) {
        fd_set rsetc = select_rset, wsetc = select_wset;

        /* according to the man, it is better to initialize it each time */
        struct timeval timeout = { .tv_sec = 6, .tv_usec = 0 };

        int nready = select(FD_SETSIZE, &rsetc, &wsetc, 0, &timeout);

        if (nready == -1) {
            return;
        }

        try {
            handleClientEvents(rsetc, wsetc, requestTimeout);
            handleUploadEvents();
            handleServerEvents(parserConf, rsetc);
            handleCgiEvents(rsetc, wsetc);
        } catch (HTTP::Exception& err) {
            handleHttpException(err);
        }
    }
}
