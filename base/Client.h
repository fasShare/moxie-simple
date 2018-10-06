#ifndef MOXIE_CLIENT_H
#define MOXIE_CLIENT_H
#include <string>

#include <Socket.h>
#include <NetAddress.h>

namespace moxie {
    
class Client {
public:
    Client(const NetAddress& addr);
private:
    NetAddress addr_;
    int sock_;
};

} // moxie

#endif 