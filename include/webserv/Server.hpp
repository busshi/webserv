#pragma once

#include <string>
#include <netinet/in.h>
#include <sys/socket.h>

#include "config-parser/ConfigParser.hpp"
#include "config-parser/ConfigItem.hpp"
#include "Header.hpp"

class Server
{
  public:
    Server(void);
    Server(Server const& src);
    ~Server(void);

    Server& operator=(Server const& rhs);

  	void init( ConfigItem * global );
  	void start(void);
    void sendResponse( Header header );
    void stop(void);

  private:
    int _port;
    int _socketFd;
    int _maxConnexion;
    int _connexion;
	std::string	_rootPath;

	int			_createSocket( void );
	sockaddr_in	_bindPort( int socketFd, int port );
	void		_listenSocket( int socketFd, int maxConnexion );
	int			_accept( int socketFd, sockaddr_in sockaddr, int addrlen );
};
