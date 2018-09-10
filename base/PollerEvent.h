#ifndef MOXIE_POLLEREVENT_H
#define MOXIE_POLLEREVENT_H
#include <iostream>
#include <sys/epoll.h>
#include <sys/poll.h>

#include <Log.h>

namespace moxie {

const uint32_t kNoneEvent = 0;
const uint32_t kReadEvent = EPOLLIN | EPOLLPRI;
const uint32_t kWriteEvent = EPOLLOUT;
const uint32_t kErrorEvent = POLLERR | POLLNVAL;
const uint32_t kOneShot = EPOLLONESHOT;

class PollerEvent {
public:
    PollerEvent() :
        event_(kNoneEvent),
        valatile_(kNoneEvent),
        sock_(-1) {
    }

    PollerEvent(int sock, uint32_t event) :
        event_(event),
        sock_(sock) {
    }

    virtual ~PollerEvent() { LOGGER_TRACE("~PollerEvent"); }

    bool ValatileErrorEvent() {
        return ((kErrorEvent) & valatile_) != 0;
    }

    bool ValatileReadEvent() {
        return ((kReadEvent) & valatile_) != 0;
    }

    bool ValatileWriteEvent() {
        return ((kWriteEvent) & valatile_) != 0;
    }

    bool ValatileCloseEvent() {
        return (valatile_ & EPOLLHUP) && !(valatile_ & kReadEvent);
    }

    void SetBits(uint32_t bits) {
        event_ |= bits;
    }

    void ClearBits(uint32_t bits) {
        event_ &= ~bits;
    }

    void EnableRead() {
        SetBits(kReadEvent);
    }

    void DisableRead() {
        ClearBits(kReadEvent);
    }

    void EnableWrite() {
        SetBits(kWriteEvent);
    }

    void DisableWrite() {
        ClearBits(kWriteEvent);
    }

    uint32_t GetEvents() const {
        return event_;
    }

    uint32_t GetValatileEvents() const {
        return valatile_;
    }

    void SetValatileEvents(uint32_t event) {
        valatile_ = event;
    }

    void SetEvents(uint32_t event) {
        event_ = event;
    }

    void SetFd(int fd) {
        sock_ = fd;
    }

    int GetFd() {
        return sock_;
    }
protected:
    uint32_t event_;
    uint32_t valatile_;
    uint32_t sock_;
};

}

#endif // MOXIE_POLLEREVENT_H
