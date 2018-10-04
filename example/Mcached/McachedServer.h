#ifndef MOXIE_MECACHEDSERVER_H
#define MOXIE_MECACHEDSERVER_H
#include <memory>
#include <unistd.h>

#include <EventLoop.h>
#include <PollerEvent.h>
#include <ListenHadler.h>
#include <NetAddress.h>
#include <McachedRequestHandler.h>
#include <IdGenerator.h>

namespace moxie {

class McachedServer : public ListenHadler {
public:
    virtual void AfterAcceptSuccess(const std::shared_ptr<PollerEvent>& client, EventLoop *loop, const std::shared_ptr<moxie::NetAddress>& cad) override;
};

}

#endif 