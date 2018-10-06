#include <assert.h>

#include <Client.h>

moxie::Client::Client(const NetAddress& addr)
    : addr_(addr),
    sock_(::socket(AF_INET, SOCK_STREAM, 0)) {
    assert(sock_ > 0);
}