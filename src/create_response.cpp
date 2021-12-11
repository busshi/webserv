#include "Directives.hpp"
#include "cgi/cgi.hpp"
#include "config/ConfigParser.hpp"
#include "core.hpp"
#include "http/message.hpp"
#include "utils/ErrorPageGenerator.hpp"
#include "utils/Logger.hpp"
#include "utils/os.hpp"

#include <dirent.h>
#include <sstream>

using std::ifstream;
using std::string;
using std::stringstream;

ConfigItem*
selectServer(std::vector<ConfigItem*>& candidates, const string& host)
{
    string serverName;
    ConfigItem* serverNameItem = 0;
    string strippedHost = host.substr(0, host.find(':'));

    for (std::vector<ConfigItem*>::iterator it = candidates.begin();
         it != candidates.end();
         ++it) {
        serverNameItem = (*it)->findNearest("server_name");
        if (serverNameItem && serverNameItem->getValue() == strippedHost) {
            return *it;
        }
    }

    return candidates.front();
}

static void
noAutoIndexResponse(string path,
                    HTTP::Request& req,
                    HTTP::Response& res,
                    Directives& directives)
{

    ifstream ifs;

    ifs.open(path.c_str());
    if (!ifs) {
        ErrorPageGenerator errorGen;
        string errorPage;

        if (directives.getRoot() == "none") {
            res.setStatus(HTTP::INTERNAL_SERVER_ERROR);
            errorPage = errorGen.checkErrorPage(
              directives.getDefaultErrorFile(),
              "500",
              "Internal Server Error",
              "the server encountered an internal error");

        } else {
            res.setStatus(HTTP::NOT_FOUND);
            errorPage = errorGen.checkErrorPage(
              directives.getDefaultErrorFile(),
              "404",
              "Not Found",
              "the page you are looking for does not exist");
        }

        res.sendFile(ERROR_PAGE);
    } else {
        // look for cgi extension
        if (!directives.getCgiExecutable().empty()) {
            for (std::vector<string>::const_iterator cit =
                   directives.getCgiExtensions().begin();
                 cit != directives.getCgiExtensions().end();
                 ++cit) {
                // cgi let's go
                if (hasFileExtension(path, *cit)) {
                    int csock = res.getClientSocket();

                    CommonGatewayInterface* cgi =
                      new CommonGatewayInterface(csock,
                                                 requests[csock],
                                                 directives.getCgiExecutable(),
                                                 path);

                    cgis[csock] = cgi;

                    // in case we're dealing with a chunked body CGI must be
                    // started when it gets unchunked
                    if (!req.isBodyChunked()) {
                        cgi->start();
                    }

#ifdef LOGGER
                    glogger << Logger::DEBUG << "CGI started for fd " << csock
                            << "\n";
#endif

                    return;
                }
            }
        }

        res.sendFile(path);
    }
}

static void
autoIndexResponse(string path, HTTP::Request& req, HTTP::Response& res)
{

    stringstream buf;

#ifdef LOGGER
    glogger << Logger::DEBUG << Logger::getTimestamp() << PURPLE
            << " Autoindex is ON... Listing directory: " << path << CLR << "\n";
#endif

    DIR* folder = opendir(path.c_str());

    if (folder) {

        struct dirent* dir;

        while ((dir = readdir(folder)) != NULL) {
            buf << "<a href=\"http://" << req.getHeaderField("Host")
                << req.getLocation() << (req.getLocation() == "/" ? "" : "/")
                << dir->d_name << "\">" << dir->d_name << "</a><br/>"
                << std::endl;
#ifdef LOGGER
            glogger << Logger::DEBUG << dir->d_name << "\n";
#endif
        }

        res.send(buf.str());

#ifdef LOGGER
        glogger << Logger::DEBUG << "\n";
#endif
        closedir(folder);
    }
}

void
createResponse(HTTP::Request& req, HTTP::Response& res, ConfigItem* server)
{
    Directives directives;
    string location;
    bool haveLoc = false;

    std::vector<ConfigItem*> locations = server->findBlocks("location");

    for (std::vector<ConfigItem*>::iterator it = locations.begin();
         it != locations.end();
         it++) {

        location = (*it)->getValue();

        if ((haveLoc = directives.haveLocation(req.getLocation(), location)) ==
            true) {
#ifdef LOGGER
            glogger << Logger::DEBUG << Logger::getTimestamp()
                    << " haveLoc is true\n";
#endif
            directives.getConfig(*it, req.getLocation());
            break;
        }
    }

    if (haveLoc == false) {
#ifdef LOGGER
        glogger << Logger::DEBUG << " haveLoc is false\n";
#endif
        directives.getConfig(server, req.getLocation());
    }

#ifdef LOGGER
    glogger << Logger::DEBUG << Logger::getTimestamp()
            << " Path: " << directives.getPath() << "\n";
    glogger << Logger::DEBUG << Logger::getTimestamp()
            << " Root: " << directives.getRoot() << "\n";
#endif
    // if upload_store is defined then upload is possible

    if (req.getMethod() == "POST" && !directives.getUploadStore().empty()) {

        std::cout << "New uploader" << std::endl;

        uploaders[req.getClientFd()] = new FileUploader(&req);

        return;
    }

    if (isFolder(directives.getPath()) == true) {
#ifdef LOGGER
        glogger << Logger::DEBUG << " IS FOLDER\n";
#endif
        directives.setPathWithIndex();

#ifdef LOGGER
        glogger << Logger ::DEBUG << Logger::getTimestamp() << " Path+index ["
                << directives.getPath() << "]\n";
#endif

        if (isFolder(directives.getPath()) == true) {

            if (directives.getAutoIndex() == "on")
                autoIndexResponse(directives.getPath(), req, res);
            else {
                ErrorPageGenerator errorGen;

                string errorPage = errorGen.checkErrorPage(
                  directives.getDefaultErrorFile(),
                  "403",
                  "Forbidden",
                  "the access is permanently forbidden");

#ifdef LOGGER
                glogger << Logger::DEBUG << Logger::getTimestamp()
                        << "Error page: " << errorPage << "\n";
#endif

                res.setHeaderField("Content-Type", "text/html");
                res.setStatus(HTTP::FORBIDDEN).sendFile(errorPage);
            }
        } else
            noAutoIndexResponse(directives.getPath(), req, res, directives);
    } else {
        noAutoIndexResponse(directives.getPath(), req, res, directives);
    }

    res.setHeaderField("Last-Modified", getLastModified(directives.getPath()));
    res.setHeaderField("Date", getDate(time(0)));
    res.setHeaderField("Location", req.getLocation());

    // add referer

    /*
_response = _headerParam["HTTP"] + " " + _headerParam["Status-Code"] +
            "\r\n" + "Content-Type: " + _headerParam["Content-Type"] +
            ";charset=UTF-8\r\n" +
            "Content-Length: " + _headerParam["Content-Length"] + "\r\n" +
            "Date: " + getDate(time(0)) + "\r\n" +
            "Last-Modified: " + getLastModified(directives.getPath()) +
            "\r\n" + "Location: " + _headerParam["Referer"] + "\r\n" +
            "Server: webserv" + "\r\n\r\n" + buf.str();
    */
}
