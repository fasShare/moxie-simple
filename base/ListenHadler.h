#ifndef MOXIE_LISTENHADLER_H
#define MOXIE_LISTENHADLER_H
#include <Handler.h>

namespace moxie {

class ListenHadler : virtual public Handler {
public:
    virtual ~ListenHadler() {}
    virtual void Process(const std::shared_ptr<PollerEvent>& event, EventLoop *loop);
    virtual void DoListen(const std::shared_ptr<PollerEvent>& event, EventLoop *loop);
    virtual void AfterAcceptSuccess(const std::shared_ptr<PollerEvent>& event, EventLoop *loop) = 0;
};

}
#endif // MOXIE_LISTENHADLER_H
