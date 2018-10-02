#include <HttpServer.h>

void moxie::HttpServer::AfterAcceptSuccess(const std::shared_ptr<PollerEvent>& client, EventLoop *loop, const std::shared_ptr<moxie::NetAddress>& cad) {
    if (!loop->Register(client, std::make_shared<HttpClientHandler>(client, cad))) {
        ::close(client->GetFd());
        return;
    }
}
