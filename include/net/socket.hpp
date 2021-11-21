#pragma once
#include <netinet/in.h>
#include <string>
#include <arpa/inet.h>
#include <list>
#include <map>

namespace Net {

class Socket {
    protected:
        int _fd;

    public:
        Socket& open(void);
        Socket& close(void);
        int getFd(void) const;
        
        Socket(void);
        Socket(const Socket& other);
        Socket& operator=(const Socket& rhs);

        virtual ~Socket(void);
};

class ClientSocket: public Socket {
    struct sockaddr_in _address;
    size_t _bufferSize;
    char* _buf;

    public:
        ClientSocket(int fd, size_t bufferSize = 1024);
        ClientSocket(const ClientSocket& other);
        ~ClientSocket(void);

        ClientSocket& operator=(const ClientSocket& rhs);
        std::string recv(size_t max_read = 0);
        ClientSocket& send(const std::string& s);
};

class ServerSocket: public Socket {
    std::list<ClientSocket> _connections;
    struct sockaddr_in _address;

    public:
        ServerSocket(void);
        ServerSocket(const ServerSocket& other);
        ServerSocket& operator=(const ServerSocket& rhs);
        ~ServerSocket(void);

        ServerSocket& bind(unsigned short port, const std::string& ip = "0.0.0.0");
        ServerSocket& listen(size_t connectionLimit = 1024);
        ClientSocket waitForConnection(void);
};

class SocketSet {
    fd_set _set;
    std::map<int, Socket*> _sockets;

    public:
        SocketSet(void);
        SocketSet(const SocketSet& other);
        ~SocketSet(void);

        SocketSet& operator=(const SocketSet& rhs);
        SocketSet& operator+=(Socket& sock);
        SocketSet& operator-=(Socket& sock);

        bool has(const Socket& sock) const;
        const fd_set& getFdSet(void) const;

        std::list<Net::Socket*> select(void);
};

}