#ifndef MOXIE_SOCKETNETADDRESS_H
#define MOXIE_SOCKETNETADDRESS_H
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string>

namespace moxie {

class NetAddress {
public:
    NetAddress(ushort sa_family = AF_INET, int sa_port = 8899, const char *sa_ip = "127.0.0.1");
    struct sockaddr *addrPtr();
    const struct sockaddr *addrPtr() const;
    socklen_t addrLen() const;
    const std::string& getIp() const;
    int getPort() const;
private:
    sa_family_t family_;
    union {
        struct sockaddr_in addr_;
        struct sockaddr_in6 addr6_;
    };
    std::string ip_;
    int port_;
};

}
#endif // MOXIE_SOCKETNETADDRESS_H
