#include "net/socket.hpp"
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdexcept>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <sys/errno.h>
#include <unistd.h>
#include <list>

/********************************************************************************/
/*                                  Socket                                      */
/********************************************************************************/

/**
 * @brief Construct a new Net:: Socket:: Socket object
 * An unbound socket is created.
 */

Net::Socket::Socket(void): _fd(-1)
{

}

Net::Socket::Socket(const Net::Socket& other)
{
    *this = other;
}

Net::Socket& Net::Socket::operator=(const Socket &rhs)
{
    if (this != &rhs) {
        _fd = rhs._fd;
    }

    return *this;
}

/**
 * @brief Destroy the Net:: Socket:: Socket object
 * The file descriptor corresponding to the socket is closed on destruction.
 */

#include <iostream>

Net::Socket::~Socket(void)
{
    if (_fd != -1) {
        std::cout << "closed!" << std::endl;
        close();
    }
}

Net::Socket& Net::Socket::open(void)
{
   

    if (_fd == -1) {
        throw std::runtime_error(std::string("Socket open: ") + strerror(errno));
    }

    fcntl(_fd, O_NONBLOCK);

    return *this;
}

Net::Socket& Net::Socket::close(void)
{
    if (_fd != -1) {
        ::close(_fd);
    }

    return *this;
}

int Net::Socket::getFd(void) const
{
    return _fd;
}

bool operator==(const Net::Socket& lhs, const Net::Socket& rhs)
{
    return lhs.getFd() == rhs.getFd();
}

bool operator!=(const Net::Socket& lhs, const Net::Socket& rhs)
{
    return !(lhs == rhs);
}

/********************************************************************************/
/*                               ServerSocket                                   */
/********************************************************************************/

Net::ServerSocket::ServerSocket(void): Socket()
{
}

Net::ServerSocket::ServerSocket(const ServerSocket& other): Socket()
{
    *this = other;
}

Net::ServerSocket::~ServerSocket(void)
{

}

Net::ServerSocket& Net::ServerSocket::operator=(const Net::ServerSocket &rhs)
{
    if (this != &rhs) {
        Socket::operator=(rhs);
        _connections = rhs._connections;
        _address = rhs._address;
    }

    return *this;
}

Net::ServerSocket& Net::ServerSocket::listen(size_t connectionLimit)
{
    if (::listen(_fd, connectionLimit) == -1) {
        throw std::runtime_error(std::string("ServerSocket listen: ") + strerror(errno));
    }

    return *this;
}

#include <iostream>

Net::ServerSocket& Net::ServerSocket::bind(unsigned short port, uint32_t ip)
{
    _address.sin_family = AF_INET;
    _address.sin_port = htons(port);
    _address.sin_addr.s_addr = ip;

    // use <sys/socket.h>'s bind not this function
    std::cout << port << "\n";
    if (::bind(_fd, (sockaddr*)&_address, sizeof (_address)) == -1) {
        throw std::runtime_error(std::string("ServerSocket bind: ") + strerror(errno));
    }

    return *this;
}

Net::ClientSocket Net::ServerSocket::waitForConnection(void)
{
    socklen_t len = sizeof (_address);
    int connectionFd = accept(_fd, (sockaddr*)&_address, &len);

    if (connectionFd == -1) {
        throw std::runtime_error(std::string("ServerSocket accept: ") + strerror(errno));
    }

    return connectionFd;
}

/********************************************************************************/
/*                               ClientSocket                                   */
/********************************************************************************/

Net::ClientSocket::ClientSocket(int fd, size_t bufferSize): Socket(), _bufferSize(bufferSize), _buf(new char[bufferSize + 1])
{
    _fd = fd;
    fcntl(_fd, F_SETFL, O_NONBLOCK);
    socklen_t slen = sizeof(_address);
    getsockname(_fd, (struct sockaddr*)&_address, &slen);
}

Net::ClientSocket::ClientSocket(const ClientSocket& other): Socket()
{
    *this = other;
}

Net::ClientSocket::~ClientSocket(void)
{
    delete[] _buf;
}

/**
 * @brief Copy contents of rhs in lhs. CAUTION: this performs a SHALLOW (not DEEP) copy.
 * 
 * @param rhs 
 * @return Net::ClientSocket& 
 */

Net::ClientSocket& Net::ClientSocket::operator=(const ClientSocket& rhs)
{
    if (this != &rhs) {
        Socket::operator=(rhs);
        _bufferSize = rhs._bufferSize;
        _buf = rhs._buf;
    }

    return *this;
}

/**
 * @brief Wrapper around recv, puting read data in an internal buffer and returning it as a string
 * 
 * @param maxRead The number of bytes to read (at most)
 * @return std::string 
 */
 
std::string Net::ClientSocket::recv(size_t maxRead)
{
    int ret = ::recv(_fd, _buf, maxRead == 0 ? _bufferSize : maxRead, 0);
    if (ret >= 0) {
        _buf[ret] = 0;
    }

    return _buf;
}

Net::ClientSocket& Net::ClientSocket::send(const std::string& s)
{
    ::send(_fd, s.c_str(), s.size(), 0);
 
    return *this;
}

in_port_t Net::ClientSocket::getPort(void) const
{
    return ntohs(_address.sin_port);
}

/********************************************************************************/
/*                               SocketSet                                      */
/********************************************************************************/

Net::SocketSet::SocketSet(void)
{
    FD_ZERO(&_set);
}

Net::SocketSet::SocketSet(const SocketSet& other)
{
    *this = other;
}


Net::SocketSet::~SocketSet(void)
{
}

Net::SocketSet& Net::SocketSet::operator=(const SocketSet& rhs)
{
    if (this != &rhs) {
        _set = rhs._set;
    }

    return *this;
}

/**
 * @brief Set the given socket if it's not already the case
 * 
 * @param sock 
 * @return Net::SocketSet& 
 */

Net::SocketSet& Net::SocketSet::operator+=(Socket& sock)
{
    FD_SET(sock.getFd(), &_set);
    _sockets[sock.getFd()] = &sock;
    
    return *this;
}

/**
 * @brief Unset the given socket if it is set
 * 
 * @param sock 
 * @return Net::SocketSet& 
 */

Net::SocketSet& Net::SocketSet::operator-=(Socket& sock)
{
    FD_CLR(sock.getFd(), &_set);
    _sockets.erase(sock.getFd());
    
    return *this;
}

/**
 * @brief Return true if the socket is set
 * 
 * @param sock 
 * @return true 
 * @return false 
 */

bool Net::SocketSet::has(const Socket &sock) const
{
    return FD_ISSET(sock.getFd(), &_set);
}

const fd_set& Net::SocketSet::getFdSet(void) const
{
    return _set;
}

std::list<Net::Socket*> Net::SocketSet::select(void)
{
    static struct timeval timeout = { .tv_sec = 1, .tv_usec = 0 };
    std::list<Net::Socket*> ready;

    fd_set rset = _set;
    int readyN = ::select(1024, &rset, 0, 0, &timeout);
    
    for (std::map<int, Socket*>::const_iterator cit = _sockets.begin(); cit != _sockets.end() && readyN > 0; ++cit) {
        if (FD_ISSET(cit->second->getFd(), &rset)) {
            ready.push_back(cit->second);
        }
    }
    
    return ready;
}