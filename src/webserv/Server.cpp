#include "Server.hpp"
#include "cgi/cgi.hpp"
#include "http/message.hpp"
#include "logger/Logger.hpp"
#include "utils/ErrorPageGenerator.hpp"
#include "utils/os.hpp"
#include "utils/string.hpp"
#include "webserv/Directives.hpp"
#include <bits/types/struct_timeval.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <set>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

Server::Server(ConfigItem* global)
{
    FD_ZERO(&_rset); FD_ZERO(&_wset);
    std::vector<ConfigItem*> serverBlocks = global->findBlocks("server");

    _config = global;

    for (std::vector<ConfigItem*>::const_iterator it = serverBlocks.begin();
         it != serverBlocks.end();
         ++it) {

        std::vector<ConfigItem*> listens = (*it)->findBlocks("listen");

        for (size_t i = 0; i != listens.size(); i++) {

            ListenData data = parseListen(listens[i]->getValue());

            // initalize each host
            if (_hosts.find(data.port) == _hosts.end()) {
                int ssockFd = socket(AF_INET, SOCK_STREAM, 0);

                if (ssockFd == -1) {
                    perror("socket: ");
                }

                fcntl(ssockFd, F_SETFL, O_NONBLOCK);

                sockaddr_in addr;

                addr.sin_family = AF_INET;
                addr.sin_port = htons(data.port);
                addr.sin_addr.s_addr = data.v4;

                if (bind(ssockFd, (sockaddr*)&addr, sizeof(addr)) == -1) {
                    perror("bind:");
                }

                if (listen(ssockFd, 1024) == -1) {
                    perror("listen: ");
                }

                // TODO: handle max connexion
                _hosts[data.port].ssockFd = ssockFd;
                _hosts[data.port].addr = addr;

                std::cout << "bound to " << data.port << std::endl;
            }

            _hosts[data.port].candidates.push_back(*it);
        }
    }
}

Server::Server(Server const& src)
{
    *this = src;
}

Server::~Server(void)
{

    std::map<unsigned short, Socket>::iterator it, ite = _sockets.end();

    for (it = _sockets.begin(); it != ite; it++) {

        if (_sockets[it->first].socket != -1)
            close(_sockets[it->first].socket);
    }

    delete _config;
}

Server&
Server::operator=(Server const& rhs)
{
    if (this != &rhs) {
        this->_sockets = rhs._sockets;
        this->_connexion = rhs._connexion;
    }

    return *this;
}

ConfigItem*
Server::_selectServer(std::vector<ConfigItem*>& candidates,
                      const std::string& host)
{
    std::string serverName;
    ConfigItem* serverNameItem = 0;
    ConfigItem* noServerName = 0;
    std::string strippedHost = host.substr(0, host.find(':'));

    for (std::vector<ConfigItem*>::iterator it = candidates.begin();
         it != candidates.end();
         ++it) {
        serverNameItem = (*it)->findNearest("server_name");
        if (serverNameItem && serverNameItem->getValue() == strippedHost) {
            return *it;
        } else if (!serverNameItem && !noServerName) {
            noServerName = *it;
        }
    }

    return noServerName;
}

void
Server::_noAutoIndexResponse(std::string path,
                             HTTP::Response& res,
                             Directives& directives)
{

    std::ifstream ifs;

    ifs.open(path.c_str());
    if (!ifs) {
        ErrorPageGenerator errorGen;
        std::string errorPage;

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
            for (std::vector<std::string>::const_iterator cit =
                   directives.getCgiExtensions().begin();
                 cit != directives.getCgiExtensions().end();
                 ++cit) {
                // cgi let's go
                if (hasFileExtension(path, *cit)) {
                    int csock = res.getClientSocket();

                    std::cout << csock << std::endl;

                    CommonGatewayInterface* cgi = new CommonGatewayInterface(
                      csock, _rset, _wset, _reqs[csock], directives.getCgiExecutable(), path);

                    cgi->start();
                    _cgis[csock] = cgi;

                    return ;
                }
            }
        }
        
        res.sendFile(path);
    }
}

void
Server::_autoIndexResponse(std::string path,
                           HTTP::Request& req,
                           HTTP::Response& res)
{

    std::stringstream buf;

    glogger << Logger::DEBUG << Logger::getTimestamp() << PURPLE
            << " Autoindex is ON... Listing directory: " << path << CLR << "\n";

    DIR* folder = opendir(path.c_str());

    if (folder) {

        struct dirent* dir;

        while ((dir = readdir(folder)) != NULL) {
            buf << "<a href=\"http://" << req.getHeaderField("Host")
                << req.getResourceURI()
                << (req.getResourceURI() == "/" ? "" : "/") << dir->d_name
                << "\">" << dir->d_name << "</a><br/>" << std::endl;
            glogger << Logger::DEBUG << dir->d_name << "\n";
        }

        res.send(buf.str());

        glogger << Logger::DEBUG << "\n";
        closedir(folder);
    }
}

void
Server::_createResponse(HTTP::Request& req,
                        HTTP::Response& res,
                        ConfigItem* server)
{
    Directives directives;
    std::string location;
    bool haveLoc = false;

    std::vector<ConfigItem*> locations = server->findBlocks("location");

    for (std::vector<ConfigItem*>::iterator it = locations.begin();
         it != locations.end();
         it++) {

        location = (*it)->getValue();

        if ((haveLoc = directives.haveLocation(req.getResourceURI(),
                                               location)) == true) {
            glogger << Logger::DEBUG << Logger::getTimestamp()
                    << " haveLoc is true\n";
            directives.getConfig(*it, req.getResourceURI());
            break;
        }
    }

    if (haveLoc == false) {
        glogger << Logger::DEBUG << " haveLoc is false\n";
        directives.getConfig(server, req.getResourceURI());
    }

    glogger << Logger::DEBUG << Logger::getTimestamp()
            << " Path: " << directives.getPath() << "\n";
    glogger << Logger::DEBUG << Logger::getTimestamp()
            << " Root: " << directives.getRoot() << "\n";

    if (isFolder(directives.getPath()) == true) {
        glogger << Logger::DEBUG << " IS FOLDER\n";
        directives.setPathWithIndex();

        glogger << Logger ::DEBUG << Logger::getTimestamp() << " Path+index ["
                << directives.getPath() << "]\n";
        if (isFolder(directives.getPath()) == true) {

            if (directives.getAutoIndex() == "on")
                _autoIndexResponse(directives.getPath(), req, res);
            else {
                ErrorPageGenerator errorGen;

                std::string errorPage = errorGen.checkErrorPage(
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
            _noAutoIndexResponse(directives.getPath(), res, directives);
    } else {
        _noAutoIndexResponse(directives.getPath(), res, directives);
    }

    res.setHeaderField("Last-Modified", getLastModified(directives.getPath()));
    res.setHeaderField("Date", getDate(time(0)));
    res.setHeaderField("Location", req.getResourceURI());

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

/**
 * @brief How does the Server loop work:
 * 
 * For each loop cycle, the select system call selects all the file descriptors that are readable and/or writable WITHOUT BLOCKING
 * given a read and write sets of file descriptors.
 * 
 * A file descriptor MUST only be read or written if selects has set it to be.
 */

void
Server::start(void)
{
    struct timeval timeout = { .tv_sec = 6, .tv_usec = 0 }; // select timeout

    // set each server socket's fd in the fd set that will be used by select
    for (HostMap::const_iterator cit = _hosts.begin(); cit != _hosts.end();
         ++cit) {
        FD_SET(cit->second.ssockFd, &_rset);
    }

    std::cout << "> webserv is running\nHit Ctrl-C to exit." << std::endl;

    while (isWebservAlive) {
        fd_set rset = _rset, wset = _wset; // make a copy of the set that will be mutated by select
        int nready = select(FD_SETSIZE, &rset, &wset, 0, &timeout);

        if (nready == -1) {
            perror("select: ");
        }

        (void) nready;

        _handleServerEvents(rset, wset);
        _handleCGIEvents(rset, wset);
        _handleClientEvents(rset, wset);

        /*
        if (reqs.find(csock) != reqs.end()) {
            HTTP::Request& req = reqs[csock];

            (void)req;

            HTTP::Request req = reqs[csock];
            std::istringstream oss(
              req.getHeaderField("Content-Length"));
            size_t contentLength = 0;
            oss >> contentLength;

            glogger << Logger::DEBUG << Logger::getTimestamp()
                    << "Length: " << contentLength << "\n";

            if (req.body.str().size() < contentLength) {
                std::cout << contentLength - req.body.str().size()
                          << std::endl;
                std::string s =
                  csock->recv(contentLength - req.body.str().size());
                req.body << s;
                glogger << Logger::DEBUG << Logger::getTimestamp()
                        << "size: " << req.body.str().size() << "\n";
            }

            if (req.body.str().size() >= contentLength) {

                HTTP::Response res(req);

                // socket cleanup
                reqs.erase(csock);
                data.erase(csock);

                glogger << Logger::INFO << Logger::getTimestamp()
                        << "Request received on port "
                        << csock->getPort() << "\n";

                ConfigItem* server = _selectServer(
                  _entrypoints[csock->getPort()].candidates,
                  req.getHeaderField("Host"));

                // TODO: check server
                _createResponse(req, res, server);
            }
        }
    }
    */
        /*
        std::list<Net::Socket*> ready = set.select();

        for (std::list<Net::Socket*>::iterator cit = ready.begin();
             cit != ready.end();
             ++cit) {
            Net::ClientSocket* csock = dynamic_cast<Net::ClientSocket*>(*cit);

            // if this is a client socket, we need to ready the data from it to
            // build the request
            if (csock) {

                // parsing request header
                if (reqs.find(csock) == reqs.end()) {

                    data[csock] += csock->recv();

                    std::string::size_type pos = 0;
                    if ((pos = data[csock].find(HTTP::BODY_DELIMITER)) !=
                        std::string::npos) {
                        // creates a new Request object and parses the header
                        HTTP::Request& req = reqs.insert(std::make_pair(csock,
        HTTP::Request(csock, data[csock].substr(0, pos)))).first->second;

                        // parse beginning of the body
                        req.body << data[csock].substr(pos, std::string::npos);

                        req.setServerBlock(_selectServer(_hosts[csock->getPort()].candidates,
        req.getHeaderField("host")));
                    }
                }

                // handle request and parse body
                if (reqs.find(csock) != reqs.end()) {
                    HTTP::Request& req = reqs[csock];

                    (void)req;

                    HTTP::Request req = reqs[csock];
                    std::istringstream oss(
                      req.getHeaderField("Content-Length"));
                    size_t contentLength = 0;
                    oss >> contentLength;

                    glogger << Logger::DEBUG << Logger::getTimestamp() <<
        "Length: " << contentLength << "\n";

                    if (req.body.str().size() < contentLength) {
                        std::cout << contentLength - req.body.str().size()
                                  << std::endl;
                        std::string s =
                          csock->recv(contentLength - req.body.str().size());
                        req.body << s;
                        glogger << Logger::DEBUG << Logger::getTimestamp() <<
        "size: " << req.body.str().size()
                                  << "\n";
                    }

                    if (req.body.str().size() >= contentLength) {

                        HTTP::Response res(req);

                        // socket cleanup
                        reqs.erase(csock);
                        data.erase(csock);

                        glogger << Logger::INFO << Logger::getTimestamp() <<
        "Request received on port "
                                  << csock->getPort() << "\n";

                        ConfigItem* server = _selectServer(
                          _entrypoints[csock->getPort()].candidates,
                          req.getHeaderField("Host"));

                        // TODO: check server
                        _createResponse(req, res, server);
                    }
                }

                // if this is a server socket, then we need to accept a new
                // connection
            } else {
                Net::ServerSocket* ssock =
                  dynamic_cast<Net::ServerSocket*>(*cit);

                // add the new client to the socket list
                Net::ClientSocket* csock =
                  new Net::ClientSocket(ssock->waitForConnection());
                set += *csock;
            }
        }*/
    }

    // delete every server socket
}