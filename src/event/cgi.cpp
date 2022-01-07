#include "cgi/cgi.hpp"
#include "core.hpp"
#include "event.hpp"
#include "http/MessageParser.hpp"
#include "http/message.hpp"

#include <stdexcept>
#include <unistd.h>

using HTTP::MessageParser;
using HTTP::Request;
using std::map;

void
handleCgiEvents(fd_set& rsetc, fd_set& wsetc)
{
    for (map<int, CGI*>::const_iterator cit = cgis.begin(); cit != cgis.end();
         ++cit) {
        int csockfd = cit->first;
        CGI* cgi = cit->second;

        if (!cgi->hasStarted()) {
            continue;
        }

        Request* reqp = requests[csockfd];

        // we can write to cgi output

        if (FD_ISSET(cgi->getOutputFd(), &wsetc)) {
            Buffer<>& bodyBuf = reqp->body;
            if (bodyBuf.size()) {
                int n =
                  write(cgi->getOutputFd(), bodyBuf.raw(), bodyBuf.size());

                if (n >= 0) {
                    bodyBuf = bodyBuf.subbuf(n);
                }
            }
        }

        int ret = 0;

        // we have something to read from the cgi
        if (FD_ISSET(cgi->getInputFd(), &rsetc)) {
            ret = read(cgi->getInputFd(), eventBuf, BUFSIZE);

            if (ret == -1) {
                LP_CLOSE_CON(csockfd);
            }

            if (ret == 0) {
                // if header hasn't been parsed yet and pipe has been closed,
                // then a problem occured
                if (!cgi->parser->hasDataQueued() &&
                    cgi->parser->getState() ==
                      MessageParser::PARSING_HEADER_FIELD_NAME) {
                    throw HTTP::Exception(reqp,
                                          HTTP::INTERNAL_SERVER_ERROR,
                                          "The CGI process exited too early, "
                                          "probably due to a system error...");
                }
                cgi->parser->stopBodyParsing();
            }
        }

        try {
            cgi->parse(eventBuf, ret);
        } catch (HTTP::MessageParser::IllFormedException& e) {
            throw HTTP::Exception(cgi->request(), HTTP::BAD_REQUEST, e.what());
        }
    }
}
