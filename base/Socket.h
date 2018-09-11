#ifndef MOXIE_SOCKET_H
#define MOXIE_SOCKET_H
#include <sys/types.h>

namespace moxie {

class NetAddress;

namespace Socket {
    bool SetNoBlocking(int sock);
    bool SetExecClose(int sock);
    bool Bind(int sock, const NetAddress& addr);
    bool Listen(int sock, int backlog);
    bool Connect(int sock, const NetAddress& addr);
    int Accept(int sock, NetAddress& addr, bool noblockingexec);
    int Accept(int sock, bool noblockingexec);
    bool SetTcpNodelay(int sock);
    bool SetReusePort(int sock);
}

}
#endif // MOXIE_SOCKET_H
