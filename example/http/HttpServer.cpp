#include <HttpServer.h>

void moxie::HttpServer::AfterAcceptSuccess(const std::shared_ptr<PollerEvent>& client, EventLoop *loop, const std::shared_ptr<moxie::NetAddress>& cad) {
    auto handler = std::make_shared<HttpClientHandler>(client, cad);
    handler->SetMethods(method_);
    if (!loop->Register(client, handler)) {
        ::close(client->GetFd());
        return;
    }
}

void moxie::HttpServer::RegisterMethodCallback(std::string cmd, std::function<void (moxie::HttpRequest&, moxie::HttpResponse&)> callback) {
    method_[cmd] = callback;
}
