#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <atomic>

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
    if (!loop->Register(event, std::make_shared<HttpServer>())) {
        LOGGER_ERROR("Loop Register Error");
        return nullptr;
    }

    loop->Loop();

    delete loop;
    return nullptr;
}

#ifndef THREAD_NUM
#define THREAD_NUM 4
#endif 

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cout << "Usage:HttpServer 127.0.0.1 8899" << std::endl; 
        return -1;
    }

    WorkContext *ctx = new (std::nothrow) WorkContext;
    if (ctx == nullptr) {
        LOGGER_ERROR("New WorkContext failed!");
        return -1;
    }

    ctx->port = std::atoi(argv[2]);
    ctx->ip = argv[1];
    
    std::vector<pthread_t> thds; thds.reserve(THREAD_NUM);
    for (int i = 0; i < THREAD_NUM; ++i) {
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
