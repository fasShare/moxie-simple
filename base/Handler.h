#ifndef MOXIE_HANDLER_H
#define MOXIE_HANDLER_H
#include <memory>

#include <PollerEvent.h>

namespace moxie {

class EventLoop;

class Handler {
public:
    virtual ~Handler() {}
    virtual void Process(const std::shared_ptr<PollerEvent>& event, EventLoop *loop) = 0;
};

}

#endif // MOXIE_HANDLER_H
