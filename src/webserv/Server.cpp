#include "Server.hpp"
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

Server::Server(void) {}

Server::Server(Server const& src)
{
    *this = src;
}

Server::~Server(void) {}

Server&
Server::operator=(Server const& rhs)
{
    if (this != &rhs) {
        this->_port = rhs._port;
        this->_socketFd = rhs._socketFd;
        this->_maxConnexion = rhs._maxConnexion;
        this->_connexion = rhs._connexion;
    }

    return *this;
}

void Server::init( ConfigItem * global )
{
	std::vector<ConfigItem*> serverBlocks = global->findBlocks("server");

	for (std::vector<ConfigItem*>::const_iterator ite = serverBlocks.begin();
	    ite != serverBlocks.end(); ++ite) {
			
			ConfigItem *	port = (*ite)->findAtomInBlock("listen");
		
			if (port) {

				_port = atoi(port->getValue().c_str());
				std::cout << "webserv listening on port " << _port << std::endl;
			}
	}

    _maxConnexion = 10;
}

void
Server::start(void)
{
    _socketFd = socket(AF_INET, SOCK_STREAM, 0);

    if (_socketFd == -1) {
        std::cout << "Failed to create socket. errno: " << errno << " "
                  << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    sockaddr_in sockaddr;

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(_port);

    if (bind(_socketFd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
        std::cout << "Failed to bind to port " << _port << ": " << errno << " "
                  << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    if (listen(_socketFd, _maxConnexion) < 0) {
        std::cout << "Failed to listen on socket. errno: " << errno << " "
                  << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    int addrlen = sizeof(sockaddr);

    while (1) {
        _connexion =
          accept(_socketFd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);

        if (_connexion < 0) {
            std::cout << "Failed to grab connection. errno: " << errno
                      << std::endl;
            exit(EXIT_FAILURE);
        }

        char buffer[1024];
        bzero(buffer, 1024);

        int bytesread = read(_connexion, &buffer, 1024);

        if (!bytesread)
            std::cout << "nothing received..." << std::endl;
        else
            std::cout << buffer << std::endl
                      << "---------------------" << std::endl;

        parseHeader(buffer);
        createResponse();
        sendResponse();
        close(_connexion);
    }
}

void
Server::parseHeader(char buffer[])
{
    std::vector<std::string> strings;
    std::istringstream buf(buffer);
    std::string s;

    while (getline(buf, s, ' '))
        strings.push_back(s);

    for (std::vector<std::string>::iterator it = strings.begin();
         it != strings.end();
         it++) {
        unsigned id = it - strings.begin();

        if (id == 0)
            _method = *it;
        if (id == 1) {
            _path = *it;
            _path = _path.substr(1);
			std::size_t	found = _path.find_last_of('.');
			if (found != std::string::npos)
				_content = _path.substr(found + 1);
        }
    }

    display();
}

void
Server::display(void)
{
    std::cout << "method: " << _method << std::endl;
    std::cout << "path: " << _path << std::endl;
    std::cout << "Content: " << _content << std::endl;
	std::cout << "--------------------------" << std::endl;
}


void
Server::sendResponse( void )
{
    //write(_connexion, response.c_str(), response.length());
    send(_connexion, _response.c_str(), _response.size(), 0);
}

void	Server::createResponse( void ) {
    std::ifstream ifs;
    std::string path;

    if (!_path.length())
        path = "asset/index.html";
    else
        path = "asset/" + _path;

    ifs.open(path.c_str());
    if (ifs) {
        std::string s;
        std::string tmp;

        while (getline(ifs, s)) {
            tmp += s;
            tmp += "\n";
        }
        ifs.close();

        unsigned len = tmp.length();
        std::ostringstream o;
        o << len;
        std::string convertedLen(o.str());

        ifs.open(path.c_str());

		_response =
          "HTTP/1.1 200 OK\nContent-Type: " + _content + ";charset=UTF-8\nContent-Length: ";
        _response += convertedLen;
        _response += "\n\n";
        while (getline(ifs, s)) {
            _response += s;
            _response += "\n";
        }
        ifs.close();
    } else {
		
		std::ifstream		ifs_error;
		std::stringstream	buf;

		ifs_error.open("asset/error_page.html", std::ifstream::in);
		buf << ifs_error.rdbuf();
		ifs_error.close();

		_response = "HTTP/1.1 404 Not Found\nContent-Type: text/html;charset=UTF-8\nContent-Length: ";
		_response += buf.str().size();
		_response += "\n\n";
		_response += "NOT FOUND\n";
//		_response += buf.str();
	}
}

void
Server::stop(void)
{
    close(_socketFd);
}
