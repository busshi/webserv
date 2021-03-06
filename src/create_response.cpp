#include "Directives.hpp"
#include "cgi/cgi.hpp"
#include "config/ConfigParser.hpp"
#include "core.hpp"
#include "http/Exception.hpp"
#include "http/message.hpp"
#include "http/status.hpp"
#include "utils/ErrorPageGenerator.hpp"
#include "utils/Logger.hpp"
#include "utils/os.hpp"

#include <dirent.h>
#include <sstream>
#include <unistd.h>

using std::ifstream;
using std::string;
using std::stringstream;

static void
noAutoIndexResponse(string path,
                    HTTP::Request& req,
                    HTTP::Response& res,
                    Directives& directives)
{

    ifstream ifs;

    ifs.open(path.c_str());

    if (!ifs) {
        throw HTTP::Exception(&req, HTTP::NOT_FOUND);
    }

    // look for cgi extension
    if (!directives.getCgiExecutable().empty()) {
        for (std::vector<string>::const_iterator cit =
               directives.getCgiExtensions().begin();
             cit != directives.getCgiExtensions().end();
             ++cit) {
            // cgi let's go
            if (hasFileExtension(path, *cit)) {
                int csock = res.getClientSocket();

                CGI* cgi = new CGI(
                  csock, requests[csock], directives.getCgiExecutable(), path);

                cgis[csock] = cgi;

                // in case we're dealing with a chunked body CGI must be
                // started when it gets unchunked
                if (!req.isBodyChunked()) {
                    cgi->start();
                }

                return;
            }
        }
    }

    // this is not handled by cgi

    if (equalsIgnoreCase(req.getMethod(), "DELETE")) {
        // TODO: check if DELETE verb is allowed

        unlink(path.c_str());

    } else {
        res.sendFile(path);
    }
}

static void
autoIndexResponse(string path, HTTP::Request& req, HTTP::Response& res)
{

    stringstream buf;

    glogger << Logger::DEBUG << Logger::getTimestamp() << PURPLE
            << " Autoindex is ON... Listing directory: " << path << CLR << "\n";

    DIR* folder = opendir(path.c_str());

    if (folder) {

        struct dirent* dir;

        while ((dir = readdir(folder)) != NULL) {
            buf << "<a href=\"http://" << req.getHeaderField("Host")
                << req.getLocation() << (req.getLocation() == "/" ? "" : "/")
                << dir->d_name << "\">" << dir->d_name << "</a><br/>"
                << std::endl;
            glogger << Logger::DEBUG << dir->d_name << "\n";
        }

        res.send(buf.str());

        glogger << Logger::DEBUG << "\n";
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
            glogger << Logger::DEBUG << Logger::getTimestamp()
                    << " haveLoc is true\n";
            directives.getConfig(*it, req.getLocation());
            break;
        }
    }

    if (haveLoc == false) {
        glogger << Logger::DEBUG << " haveLoc is false\n";
        directives.getConfig(server, req.getLocation());
    }

    glogger << Logger::DEBUG << Logger::getTimestamp()
            << " Path: " << directives.getPath() << "\n";
    glogger << Logger::DEBUG << Logger::getTimestamp()
            << " Root: " << directives.getRoot() << "\n";
    // if upload_store is defined then upload is possible

    if (req.getMethod() == "POST" && !directives.getUploadStore().empty()) {

        std::string maxFileSize = directives.getUploadMaxFileSize();

        uploaders[req.getClientFd()] =
          new FileUploader(&req,
                           directives.getUploadStore(),
                           maxFileSize.empty() ? 0 : parseSize(maxFileSize));

        return;
    }

    if (isFolder(directives.getPath()) == true) {
        glogger << Logger::DEBUG << " IS FOLDER\n";
        directives.setPathWithIndex();

        glogger << Logger ::DEBUG << Logger::getTimestamp() << " Path+index ["
                << directives.getPath() << "]\n";

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

                glogger << Logger::DEBUG << Logger::getTimestamp()
                        << "Error page: " << errorPage << "\n";

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

