#ifndef MOXIE_MCACHEDREQUESTHANDLER_H
#define MOXIE_MCACHEDREQUESTHANDLER_H
#include <iostream>
#include <map>
#include <string>
#include <memory>

#include <ClientHandler.h>
#include <EventLoop.h>
#include <PollerEvent.h>
#include <NetAddress.h>
#include <HttpRequest.h>
#include <HttpResponse.h>

namespace moxie {

const static uint16_t STATE_HTTPREQUEST_BEGIN = (1 << 0);
const static uint16_t STATE_HTTPREQUEST_FIRSTLINE = (1 << 1);
const static uint16_t STATE_HTTPREQUEST_HEADER = (1 << 2);
const static uint16_t STATE_HTTPREQUEST_BODY = (1 << 3);
const static uint16_t STATE_HTTPREQUEST_PROCESS = (1 << 4);

class HttpClientHandler : public ClientHandler {
public:
    HttpClientHandler(const std::shared_ptr<PollerEvent>& client,  const std::shared_ptr<moxie::NetAddress>& cad);
    bool ParseHttpHeader();
    virtual void AfetrRead(const std::shared_ptr<PollerEvent>& event, EventLoop *loop) override;
    virtual void AfetrWrite(const std::shared_ptr<PollerEvent>& event, EventLoop *loop) override;
private:
    void ReplyErrorRequest(const string& error);
private:
    std::shared_ptr<PollerEvent> event_;
    std::shared_ptr<moxie::NetAddress> peer_;
    HttpRequest request_;
    HttpResponse response_;
    uint16_t state_;
};

}

#endif 