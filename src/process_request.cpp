#include "Directives.hpp"
#include "FileUploader.hpp"
#include "cgi/cgi.hpp"
#include "config/ConfigItem.hpp"
#include "core.hpp"
#include "http/Exception.hpp"
#include "http/message.hpp"
#include "http/status.hpp"

#include <dirent.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

using std::ostringstream;
using std::string;
using std::vector;

using HTTP::Request;

static ConfigItem*
selectServer(HTTP::Request* req)
{
    sockaddr_in addr;
    socklen_t slen = sizeof(addr);
    getsockname(req->getClientFd(), (sockaddr*)&addr, &slen);

    uint16_t port = ntohs(addr.sin_port);
    vector<ConfigItem*> candidates = hosts[port].candidates;
    string serverName;
    ConfigItem* serverNameItem = 0;
    string host = req->getHeaderField("Host");
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

static Directives
loadDirectives(Request* req, ConfigItem* serverBlock)
{
    Directives directives;
    vector<ConfigItem*> locations = serverBlock->findBlocks("location");
    ConfigItem* loadFrom = serverBlock;

    /* match the more specific location */

    for (size_t i = 0; i != locations.size(); ++i) {
        string locv = locations[i]->getValue().size() > 1
                        ? trimTrailing(locations[i]->getValue(), "/")
                        : locations[i]->getValue();

        if (req->getLocation().find(locv) == 0) {
            if (loadFrom == serverBlock ||
                locv.size() > loadFrom->getValue().size()) {
                loadFrom = locations[i];
            }
        }
    }

    req->setBlock(loadFrom);
    directives.load(req, loadFrom);

    // std::cout << "Path=" << directives.getPath() << std::endl;

    return directives;
}

static void
processCgiRequest(Request* req,
                  const std::string& cgiExecutablePath,
                  const std::string& scriptPath)
{
    CGI* cgi = new CGI(req->getClientFd(), req, cgiExecutablePath, scriptPath);

    cgis[req->getClientFd()] = cgi;

    // if body is chunked, we need to unchunk it before we can start the CGI
    if (!req->isBodyChunked()) {
        cgi->start();
    }
}

static void
serveFile(Request* req, const std::string& path)
{
    req->response()->sendFile(path);
}

static void
indexDirectoryContents(Request* req, const std::string& path)
{
    ostringstream buf;

    DIR* folder = opendir(path.c_str());

    if (folder) {

        struct dirent* dir;

        while ((dir = readdir(folder)) != NULL) {
            buf << "<a href=\"http://" << req->getHeaderField("Host")
                << req->getLocation() << (req->getLocation() == "/" ? "" : "/")
                << dir->d_name << "\">" << dir->d_name << "</a><br/>"
                << std::endl;
            glogger << Logger::DEBUG << dir->d_name << "\n";
        }

        req->response()->setHeaderField("Content-Length",
                                        ntos(buf.str().size()));
        req->response()->send(buf.str());

        glogger << Logger::DEBUG << "\n";
        closedir(folder);
    }
}

static void
processUploadPost(Request* req,
                  const std::string& path,
                  unsigned long long maxUploadFileSize)
{
    FileUploader* uploader = new FileUploader(req, path, maxUploadFileSize);

    uploaders[req->getClientFd()] = uploader;
}

static void
processUploadDelete(Request* req, const string& path)
{
    req->response()->send("");
    unlink(path.c_str());
}

static void
checkRequestIntegrity(HTTP::Request* req, Directives& direc)
{
    if (!req->isBodyChunked()) {
        // no content-length will mean the same as Content-Length: 0
        unsigned long long cl =
          parseInt(req->getHeaderField("Content-Length"), 10);

        if (cl >= req->getMaxBodySize()) {
            throw HTTP::Exception(
              req,
              HTTP::REQUEST_PAYLOAD_TOO_LARGE,
              "Request body exceeds limit enforced by configuration");
        }
    }

    // check: forbidden methods
    vector<std::string> forbiddenMethods = direc.getForbiddenMethods();

    for (size_t i = 0; i != forbiddenMethods.size(); ++i) {
        if (equalsIgnoreCase(req->getMethod(), forbiddenMethods[i])) {
            throw HTTP::Exception(req,
                                  HTTP::METHOD_NOT_ALLOWED,
                                  "This method is forbidden on that route");
        }
    }
}

/* returns true if request has been rewritten */

static bool
performRewrite(Request* req, Directives& direc)
{
    if (!direc.getRewrite().empty()) {
        req->rewrite(direc.getRewrite());
        return true;
    }

    if (!direc.getRewriteLocation().empty()) {
        req->rewrite(direc.getRewriteLocation());
        return true;
    }

    return false;
}

void
processRequest(Request* req)
{
    ConfigItem* serverBlock = selectServer(req);
    Directives direc = loadDirectives(req, serverBlock);

    req->setMaxBodySize(direc.getBodyMaxSize());
    checkRequestIntegrity(req, direc);

    if (!direc.getRedirect().empty()) {
        req->response()->setHeaderField("Location", direc.getRedirect());
        throw HTTP::Exception(req, HTTP::MOVED_PERMANENTLY);
    }

    if (performRewrite(req, direc))
        return;

    std::string path = direc.getPath();
    struct stat s;

    if (stat(path.c_str(), &s) == -1) {
        if (errno == ENOENT) {
            throw HTTP::Exception(
              req,
              HTTP::NOT_FOUND,
              path + " does not point to a file on the local filesystem");
        }

        // any other system error => 500
        else {
            throw HTTP::Exception(
              req, HTTP::INTERNAL_SERVER_ERROR, strerror(errno));
        }
    }

    // In case it is a directory, try all the provided indexes
    if (s.st_mode & S_IFDIR && req->getMethod() == "GET") {
        vector<string> indexes = direc.getIndexes();

        for (size_t i = 0; i != indexes.size(); ++i) {
            std::string tmp = path + "/" + indexes[i];
            stat(tmp.c_str(), &s);

            // stop on first match
            if (!(s.st_mode & S_IFDIR)) {
                path = tmp;
                break;
            }
        }
    }

    // if isDir is still true, it means that no suitable index was found. Let's
    // try autoindex now.
    if (s.st_mode & S_IFDIR) {
        if (req->getMethod() == "POST" && direc.allowsUpload()) {
            processUploadPost(
              req, direc.getUploadStore(), direc.getUploadMaxFileSize());
            return;
        } else if (!direc.getAutoIndex()) {
            throw HTTP::Exception(
              req,
              HTTP::FORBIDDEN,
              "Can't list contents of the directory: autoindex is turned off");
        } else if (req->getMethod() == "GET") {
            indexDirectoryContents(req, path);
            return;
        }
    }

    if (req->getMethod() == "DELETE" && direc.allowsUpload()) {
        processUploadDelete(req, path);
        return;
    }

    if (direc.allowsCgi()) {
        vector<string> exts = direc.getCgiExtensions();

        for (size_t i = 0; i != exts.size(); ++i) {
            if (hasFileExtension(path, exts[i])) {
                processCgiRequest(req, direc.getCgiExecutable(), path);
                return;
            }
        }
    }

    if (req->getMethod() == "GET") {
        serveFile(req, path);
    } else {
        throw HTTP::Exception(
          req, HTTP::METHOD_NOT_ALLOWED, "Irrelevant operation");
    }
}
