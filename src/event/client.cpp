#include "core.hpp"
#include "event.hpp"
#include "http/message.hpp"

#include <unistd.h>

using HTTP::Request;
using HTTP::Response;
using std::map;

void
handleClientEvents(fd_set& rsetc,
                   fd_set& wsetc,
                   unsigned long long requestTimeout)
{
    for (map<int, Request*>::const_iterator cit = requests.begin();
         cit != requests.end();) {
        int csockfd = cit->first;
        Request* reqp = cit->second;

        ++cit;

        if (reqp->timer.getElapsed() > requestTimeout) {
            reqp->response()->setStatus(HTTP::REQUEST_TIMEOUT);
            LP_CLOSE_CON(csockfd);
        }

        int ret = 0;

        /* if there is something to read from the client socket */

        if (FD_ISSET(csockfd, &rsetc)) {
            ret = recv(csockfd, eventBuf, BUFSIZE, 0);

            if (ret <= 0) {
                LP_CLOSE_CON(csockfd);
            }
        }

        eventBuf[ret] = 0;
        if (!reqp->parse(eventBuf, ret)) {
            throw HTTP::Exception(
              reqp, HTTP::BAD_REQUEST, "ill-formed HTTP request");
        }

        // we can read from the file we need to serve to the client

        if (reqp->getFile() != -1 && FD_ISSET(reqp->getFile(), &rsetc)) {
            int n = read(reqp->getFile(), eventBuf, BUFSIZE);

            if (n == -1) {
                LP_CLOSE_CON(csockfd);
            }

            if (n == 0) {
                reqp->dropFile();
            }

            reqp->response()->data += Buffer<>(eventBuf, n);
        }

        /* if we can write */

        if (FD_ISSET(csockfd, &wsetc)) {
            Response* resp = reqp->response();

            if (resp) {
                Buffer<>& buf = resp->data;

                if (buf.size()) {
                    int ret = send(csockfd, buf.raw(), buf.size(), 0);

                    if (ret == -1) {
                        LP_CLOSE_CON(csockfd);
                    }

                    // let buf contain the unprocessed data
                    buf = buf.subbuf(ret);
                }

                if (!buf.size()) {
                    if (cgis.find(csockfd) != cgis.end()) {
                        if (cgis[csockfd]->isDone()) {
                            terminateRequest(reqp);
                        }
                    } else if (uploaders.find(csockfd) != uploaders.end()) {
                        if (uploaders[csockfd]->isDone()) {
                            terminateRequest(reqp);
                        }
                    } else if (reqp->isDone()) {
                        terminateRequest(reqp);
                    }
                }
            }
        }
    }
}
