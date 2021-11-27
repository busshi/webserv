#include <sys/select.h>

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