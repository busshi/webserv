#pragma once

#include <string>
#include <map>
#include <netinet/in.h>
#include <sys/socket.h>

#include "config-parser/ConfigParser.hpp"
#include "config-parser/ConfigItem.hpp"
#include "Header.hpp"

#define RED		"\033[31m"
#define GREEN	"\033[32m"
#define ORANGE	"\033[33m"
#define PURPLE	"\033[35m"
#define BOLD	"\033[1m"
#define CLR		"\033[0m"

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
	struct Socket {

		int				socket;
		int				maxConnexion;
		int				connexion;
		sockaddr_in		sockaddr;
		int				addrlen;
		std::string		root;
		//std::vector<ConfigItem*>	items;
	};

	std::map<unsigned short, Socket>	_sockets;

    int _connexion;

	int			_createSocket( void );
	sockaddr_in	_bindPort( int socketFd, unsigned short port );
	void		_listenSocket( int socketFd, int maxConnexion );
	int			_accept( int socketFd, sockaddr_in sockaddr, int addrlen );
};
