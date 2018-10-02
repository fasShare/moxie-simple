#ifndef MOXIE_HTTPSERVER_H
#define MOXIE_HTTPSERVER_H
#include <memory>
#include <unistd.h>

#include <EventLoop.h>
#include <PollerEvent.h>
#include <ListenHadler.h>
#include <NetAddress.h>
#include <HttpRequestHandler.h>

namespace moxie {

class HttpServer : public ListenHadler {
public:
    void RegisterMethodCallback(std::string cmd, std::function<void (moxie::HttpRequest&, moxie::HttpResponse&)> callback);
    virtual void AfterAcceptSuccess(const std::shared_ptr<PollerEvent>& client, EventLoop *loop, const std::shared_ptr<moxie::NetAddress>& cad) override;
private:
    std::map<std::string, std::function<void (moxie::HttpRequest&, moxie::HttpResponse&)>> method_;
};

}

#endif 