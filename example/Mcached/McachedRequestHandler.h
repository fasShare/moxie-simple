#ifndef MOXIE_MCACHEDREQUESTHANDLER_H
#define MOXIE_MCACHEDREQUESTHANDLER_H
#include <iostream>
#include <vector>
#include <string>
#include <memory>

#include <ClientHandler.h>
#include <EventLoop.h>
#include <PollerEvent.h>
#include <NetAddress.h>

namespace moxie {

class McachedClientHandler : public ClientHandler {
public:
    McachedClientHandler(const std::shared_ptr<PollerEvent>& client,  const std::shared_ptr<moxie::NetAddress>& cad);
    
    virtual void AfetrRead(const std::shared_ptr<PollerEvent>& event, EventLoop *loop) override;
    virtual void AfetrWrite(const std::shared_ptr<PollerEvent>& event, EventLoop *loop) override;
private:
    class ResetArgcArgv {
    public:
        ResetArgcArgv(size_t& argc, std::vector<std::string>& argv) 
            : argc_(argc),
            argv_(argv) {}
        ~ResetArgcArgv() {
            argc_ = 0;
            argv_.clear();
        }
    private:
            size_t& argc_;
            std::vector<std::string>& argv_;
    };

    bool DoMcachedCammand();
    bool ParseRedisRequest();
    void DebugArgcArgv() const;
    void ReplyString(const std::string& error);
    void ReplyBulkString(const std::string& error);
private:
    std::shared_ptr<PollerEvent> event_;
    std::shared_ptr<moxie::NetAddress> peer_;
    size_t argc_;
    std::vector<std::string> argv_;
    int curArgvlen_;
};

}

#endif 