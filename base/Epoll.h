#ifndef MOXIE_EPOLL_H
#define MOXIE_EPOLL_H
#include <sys/epoll.h>
#include <vector>
#include <unistd.h>

#include <Timestamp.h>
#include <PollerEvent.h>

namespace moxie {

class Epoll {
public:
    Epoll();
    ~Epoll() {
        ::close(epoll_fd_);
    }

    bool EventAdd(int fd, struct epoll_event* event);
    bool EventDel(int fd, struct epoll_event* event);
    bool EventMod(int fd, struct epoll_event* event);
    int LoopWait(struct epoll_event* events, int maxevents, int timeout);
    bool EventCtl(int op, int fd, struct epoll_event* event);
private:
    int epoll_fd_;
    std::vector<struct epoll_event> revents_;
    int maxNum_;
    const int addStep_;
};

}
#endif // MOXIE_EPOLL_H
