#pragma once

#include <string>
#include <vector>
#include "config-parser/ConfigParser.hpp"
#include "config-parser/ConfigItem.hpp"

class Server
{
  public:
    Server(void);
    Server(Server const& src);
    ~Server(void);

    Server& operator=(Server const& rhs);

//    void init(std::vector<ConfigItem*>);
  	void init( ConfigItem * global );
  	void start(void);
    void parseHeader(char buffer[]);
    void createResponse(void);
    void sendResponse(void);
    void stop(void);

    void display(void);

  private:
    int _port;
    int _socketFd;
    int _maxConnexion;
    int _connexion;

    std::string _method;
    std::string _path;
    std::string _content;
    std::string _response;
};
