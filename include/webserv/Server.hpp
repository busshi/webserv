#pragma once

#include <string>
#include <vector>
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

    void display(void);

  private:
    int _port;
    int _socketFd;
    int _maxConnexion;
    int _connexion;
};
