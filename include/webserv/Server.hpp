#pragma once

#include <string>
#include <map>
#include <netinet/in.h>
#include <sys/socket.h>

#include "config-parser/ConfigParser.hpp"
#include "config-parser/ConfigItem.hpp"
#include "Header.hpp"
#include "Constants.hpp"

class Server
{
  public:
    Server(ConfigItem * global);
    Server(Server const& src);
    ~Server(void);

    Server& operator=(Server const& rhs);

 // 	void init( ConfigItem * global );
  	void start(void);
    void sendResponse( Header header );
   // void stop(void);

  private:
	struct Socket {

		int				socket;
		uint32_t		ipv4;
		int				maxConnexion;
		int				connexion;
		sockaddr_in		sockaddr;
		int				addrlen;
		//std::string		root;
		//std::string		location;
		//std::string		autoindex;
		//std::vector<std::string>		indexes;
		ConfigItem*		item;
	};

	std::map<unsigned short, Socket>	_sockets;

//	struct	Config {

//		std::string		root;
//		std::string		location;
//		std::string		autoindex;
//		std::vector<std::string>		indexes;
//	};

//	std::vector<Config>	_parsed;

    int _connexion;
	
	ConfigItem* _config;

	int			_createSocket( void );
	sockaddr_in	_bindPort( int socketFd, unsigned short port, uint32_t ipv4 );
	void		_listenSocket( int socketFd, int maxConnexion );
	int			_accept( int socketFd, sockaddr_in sockaddr, int addrlen );
};

extern bool isWebservAlive;
