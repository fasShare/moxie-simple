#ifndef MOXIE_CLIENTHANDLER_H
#define MOXIE_CLIENTHANDLER_H
#include <Handler.h>
#include <Buffer.h>
#include <Log.h>

namespace moxie {

class ClientHandler : virtual public Handler{
public:
    ClientHandler();
    virtual ~ClientHandler() { LOGGER_TRACE("~ClientHandler"); }
    virtual void Process(const std::shared_ptr<PollerEvent>& event, EventLoop *loop);
    virtual void DoRead(const std::shared_ptr<PollerEvent>& event, EventLoop *loop);
    virtual void DoWrite(const std::shared_ptr<PollerEvent>& event, EventLoop *loop);
    virtual void AfetrRead(const std::shared_ptr<PollerEvent>& event, EventLoop *loop) = 0;
    virtual void AfetrWrite(const std::shared_ptr<PollerEvent>& event, EventLoop *loop) = 0;
protected:
    Buffer readBuf_;
    Buffer writeBuf_;
};

}

#endif // CLIENTHANDLER_H
