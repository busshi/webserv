#pragma once

#include <string>
#include <map>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <set> 

#include "net/socket.hpp"
#include "config-parser/ConfigParser.hpp"
#include "config-parser/ConfigItem.hpp"
#include "Constants.hpp"
#include "http/message.hpp"
#include "webserv/Directives.hpp"
#include "cgi/cgi.hpp"

class Server
{
  public:
    Server(ConfigItem * global);
    Server(Server const& src);
    ~Server(void);

    Server& operator=(Server const& rhs);

  	void start(void);

  private:
	struct Socket {

		int				socket;
		uint32_t		ipv4;
		int				maxConnexion;
		int				connexion;
		sockaddr_in		sockaddr;
		int				addrlen;
		ConfigItem*		item;
	};

	/**
	 * @brief Representation of a host
	 * candiates are all the blocks that are connected to this host.
	 */

	struct Host {
		int ssockFd;
		sockaddr_in addr;
		std::vector<ConfigItem*> candidates;
		std::vector<CommonGatewayInterface*> cgis;
	};

	std::map<int, std::string> _data;
    std::map<int, HTTP::Request> _reqs;
    std::map<int, CommonGatewayInterface*> _cgis;
    std::set<int> _clients;

	fd_set _rset, _wset;

	std::map<unsigned short, Socket>	_sockets;
	
	typedef std::map<unsigned short, Host> HostMap;
	HostMap _hosts;

    int _connexion;
	
	ConfigItem* _config;

	int			_createSocket( void );
	sockaddr_in	_bindPort( int socketFd, unsigned short port, uint32_t ipv4 );
	void		_listenSocket( int socketFd, int maxConnexion );
	int			_accept( int socketFd, sockaddr_in sockaddr, int addrlen );

	ConfigItem* _selectServer(std::vector<ConfigItem*>& candidates, const std::string& host);
	void _createResponse(HTTP::Request& req, HTTP::Response& res, ConfigItem* server);
	
	
	void _noAutoIndexResponse( std::string path, HTTP::Response& res, Directives& direc);
	//void _autoIndexResponse( std::string path, std::stringstream & buf, HTTP::Request& req, HTTP::Response& res);
	void _autoIndexResponse( std::string path, HTTP::Request& req, HTTP::Response& res);

	// TBD: move below functions inside their own class
	void _genErrorPage( std::string file, std::string code, std::string msg, std::string sentence );
	std::string	_replace(std::string in, std::string s1, std::string s2);
	std::string _checkErrorPage( std::string defaultPage, std::string code, std::string errorMsg, std::string errorSentence);

	void _handleClientEvents(const fd_set& rset, const fd_set& wset);
	void _handleServerEvents(const fd_set& rset, const fd_set& wset);
	void _handleCGIEvents(const fd_set& rset, const fd_set& wset);
	void _closeConnection(int sockFd);
};

extern bool isWebservAlive;
