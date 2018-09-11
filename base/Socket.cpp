#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

#include <Socket.h>
#include <Log.h>
#include <NetAddress.h>

bool moxie::Socket::SetNoBlocking(int sock) {
    int flag = ::fcntl(sock, F_GETFL);
    flag |= O_NONBLOCK;
    int ret = ::fcntl(sock, F_SETFL, flag);
    if (ret == -1) {
        LOGGER_SYSERR("fcntl error : " << ::strerror(errno));
        return false;
    }
    return true;
}

bool moxie::Socket::SetExecClose(int sock) {
    int flag = ::fcntl(sock, F_GETFD, 0);
    flag |= FD_CLOEXEC;
    int ret = ::fcntl(sock, F_SETFD, flag);
    if (ret == -1) {
        LOGGER_SYSERR("fcntl error : " << ::strerror(errno));
        return false;
    }
    return true;
}

bool moxie::Socket::Bind(int sock, const NetAddress& addr) {
    int ret = ::bind(sock, addr.addrPtr(), addr.addrLen());
    if (ret == -1) {
        LOGGER_SYSERR("bind error : " << ::strerror(errno));
        return false;
    }
    return true;
}

bool moxie::Socket::Listen(int sock, int backlog) {
    int ret = ::listen(sock, backlog);
    if (ret == -1) {
        LOGGER_SYSERR("listen error : " << ::strerror(errno));
        return false;
    }
    return true;
}

bool moxie::Socket::Connect(int sock, const NetAddress& addr) {
    int ret = ::connect(sock, addr.addrPtr(), addr.addrLen());
    if (ret == -1) {
        if (errno == EINPROGRESS) {
            return true;
        }
        LOGGER_SYSERR("connect error : " << ::strerror(errno));
        return false;
    }
    return true;
}

int moxie::Socket::Accept(int sock, moxie::NetAddress& addr, bool noblockingexec) {
    int flags = 0;
    if (noblockingexec) {
        flags = SOCK_NONBLOCK | SOCK_CLOEXEC;
    }
    socklen_t len = addr.addrLen();
    int ret = ::accept4(sock, addr.addrPtr(), &len, flags);
    if(ret == -1) {
        LOGGER_SYSERR("accept error : " << ::strerror(errno));
        return ret;
    }
    return ret;
}

int moxie::Socket::Accept(int sock, bool noblockingexec) {
    int flags = 0;
    if (noblockingexec) {
        flags = SOCK_NONBLOCK | SOCK_CLOEXEC;
    }
    int ret = ::accept4(sock, nullptr, nullptr, flags);
    if(ret == -1) {
        LOGGER_SYSERR("accept error : " << ::strerror(errno));
        return ret;
    }
    return ret;
}

bool moxie::Socket::SetTcpNodelay(int sock) {
    int enable = 1;
    if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable)) < 0) {
        LOGGER_SYSERR("setsockopt error : " << ::strerror(errno));
        return false;
    }
    return true;
}

bool moxie::Socket::SetReusePort(int sock) {
    int enable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (void*)&enable, sizeof(enable)) < 0) {
        LOGGER_SYSERR("setsockopt error : " << ::strerror(errno));
        return false;
    }
    return true;
}
