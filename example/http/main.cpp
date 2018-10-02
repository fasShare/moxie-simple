#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <atomic>
#include <functional>

#include <EventLoop.h>
#include <string.h>
#include <Log.h>
#include <Socket.h>
#include <NetAddress.h>
#include <PollerEvent.h>
#include <HttpServer.h>

using namespace moxie;

struct WorkContext {
    short port;
    std::string ip;
};

void ProcessGet(HttpRequest& request, HttpResponse& response) {
    response.SetScode("200");
    response.SetStatus("OK");
    response.SetVersion(request.GetVersion());
    
    std::string content = "<html><body>  GET Request OK!  </body></html>";
    response.AppendBody(content.c_str(), content.size());

    response.PutHeaderItem("Content-Type", "text/html");
    response.PutHeaderItem("Content-Length", std::to_string(content.size()));
}

void *StartMcached(void *data) {
    if (!data) {
        LOGGER_ERROR("Data is nulptr!");
        return nullptr;
    }

    WorkContext *ctx = (WorkContext *)data;

    EventLoop *loop = new EventLoop();
    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) {
        LOGGER_ERROR("socket error : " << strerror(errno));
        return nullptr;
    }

    Socket::SetExecClose(server);
    Socket::SetNoBlocking(server);
    Socket::SetReusePort(server);

    NetAddress addr(AF_INET, ctx->port, ctx->ip.c_str());

    if (!(Socket::Bind(server, addr)
          && Socket::Listen(server, 128))) {
        return nullptr;
    }

    // 注册监听套接字的处理类
    std::shared_ptr<PollerEvent> event = std::make_shared<PollerEvent>(server, moxie::kReadEvent);
    auto http_server = std::make_shared<HttpServer>();
    
    http_server->RegisterMethodCallback("GET", ProcessGet);
   
    if (!loop->Register(event, http_server)) {
        LOGGER_ERROR("Loop Register Error");
        return nullptr;
    }

    loop->Loop();

    delete loop;
    return nullptr;
}

int main(int argc, char **argv) {
    if (argc < 4) {
        std::cout << "Usage:HttpServer 127.0.0.1 8899 thread_num" << std::endl; 
        return -1;
    }

    WorkContext *ctx = new (std::nothrow) WorkContext;
    if (ctx == nullptr) {
        LOGGER_ERROR("New WorkContext failed!");
        return -1;
    }

    ctx->port = std::atoi(argv[2]);
    ctx->ip = argv[1];
    
    int thread_num = std::atoi(argv[3]);
    assert(thread_num > 0);
    std::vector<pthread_t> thds; thds.reserve(static_cast<size_t>(thread_num));
    for (int i = 0; i < thread_num; ++i) {
        pthread_t td;
        if (pthread_create(&td, nullptr, StartMcached, ctx) < 0) {
            LOGGER_ERROR("Pthread_create failed!");
            continue;
        }
        thds.push_back(td);
    }

    for (size_t i = 0; i < thds.size(); ++i) {
        pthread_join(thds[i], nullptr);
    }

    delete ctx;
    return 0;
}
