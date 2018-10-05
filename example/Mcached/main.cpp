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
#include <EventLoopThread.h>
#include <PollerEvent.h>
#include <McachedServer.h>
#include <mraft/floyd/include/floyd.h>

using namespace moxie;
using namespace floyd;

Floyd *floyd_raft;

struct WorkContext {
    short port;
    std::string ip;
};

void RegisterMcachedWorkerTask(EventLoop *loop, WorkContext *ctx) {
    if (!ctx) {
        LOGGER_ERROR("Ctx is nulptr!");
        return;
    }

    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) {
        LOGGER_ERROR("socket error : " << strerror(errno));
        return;
    }

    Socket::SetExecClose(server);
    Socket::SetNoBlocking(server);
    Socket::SetReusePort(server);

    NetAddress addr(AF_INET, ctx->port, ctx->ip.c_str());

    if (!(Socket::Bind(server, addr)
          && Socket::Listen(server, 128))) {
        return;
    }

    // 注册监听套接字的处理类
    std::shared_ptr<PollerEvent> event = std::make_shared<PollerEvent>(server, moxie::kReadEvent);
    if (!loop->Register(event, std::make_shared<McachedServer>())) {
        LOGGER_ERROR("Loop Register Error");
        return;
    }
}

#ifndef THREAD_NUM
#define THREAD_NUM 4
#endif 

int main(int argc, char **argv) {
    if (argc < 6) {
        std::cout << "Usage:mcached 127.0.0.1:8901 127.0.0.1 8901 ./data1/ 6379" << std::endl; 
        return -1;
    }
    Options op(argv[1], argv[2], atoi(argv[3]), argv[4]);
    op.Dump();

    slash::Status s;
    s = Floyd::Open(op, &floyd_raft);
    std::cout << s.ToString().c_str() << std::endl;

    WorkContext *ctx = new (std::nothrow) WorkContext;
    if (ctx == nullptr) {
        LOGGER_ERROR("New WorkContext failed!");
        return -1;
    }

    ctx->port = std::atoi(argv[5]);
    ctx->ip = "127.0.0.1";
    
    std::vector<std::shared_ptr<EventLoopThread>> thds; thds.reserve(THREAD_NUM);
    for (int i = 0; i < THREAD_NUM; ++i) {
        EventLoop *loop = new EventLoop;
        if (loop && ctx) {
            auto td = std::make_shared<EventLoopThread>();
            td->Start();
            td->PushTask(std::bind(RegisterMcachedWorkerTask, std::placeholders::_1, ctx));
            thds.push_back(td);
        }
    }

    for (size_t i = 0; i < thds.size(); ++i) {
        thds[i]->Join();
    }

    delete ctx;
    return 0;
}
