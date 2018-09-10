#ifndef MOXIE_EVENTLOOP_H
#define MOXIE_EVENTLOOP_H
#include <memory>
#include <assert.h>
#include <unordered_map>

#include <PollerEvent.h>
#include <SigIgnore.h>
#include <Handler.h>
#include <Epoll.h>
#include <Mutex.h>

namespace moxie {

class EventLoop {
public:
    EventLoop() :
        epoll_(new (std::nothrow) Epoll()),
        quit_(false) {
        if (!epoll_) {
            assert(false);
        }
    }

    bool Register(const std::shared_ptr<PollerEvent>& event, const std::shared_ptr<Handler>& handler);
    bool Modity(const std::shared_ptr<PollerEvent>&  event);
    bool Delete(const std::shared_ptr<PollerEvent>& event);
    void Loop();
    void Quit();
private:
    struct EventContext {
        int fd;
        std::shared_ptr<PollerEvent> event;
        std::shared_ptr<Handler> handle;
    };
private:
    Epoll *epoll_;
    std::unordered_map<int, EventContext*> contexts_;
    bool quit_;
    SigIgnore ignore_;
    static size_t kEpollRetBufSize;
    static size_t kDefaultTimeOut;
};

}

#endif // MOXIE_EVENTLOOP_H
