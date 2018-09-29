#include <iostream>
#include <string>
#include <thread>
#include <memory>
#include <pthread.h>

#include <EventLoop.h>
#include <string.h>
#include <Log.h>
#include <Socket.h>
#include <NetAddress.h>
#include <PollerEvent.h>
#include <ListenHadler.h>
#include <ClientHandler.h>

using namespace moxie;

class EchoClientHandler : public ClientHandler {
public:
    virtual void AfetrRead(const std::shared_ptr<PollerEvent>& event, EventLoop *loop) {
        if (readBuf_.readableBytes() > 0) {
            writeBuf_.append(readBuf_.peek(), readBuf_.readableBytes());
            readBuf_.retrieveAll();
            event->EnableWrite();
            loop->Modity(event);
        }
    }

    virtual void AfetrWrite(const std::shared_ptr<PollerEvent>& event, EventLoop *loop) {
        if (writeBuf_.readableBytes() > 0) {
            return;
        }
        event->DisableWrite();
        loop->Modity(event);
    }
};

class Echo : public ListenHadler {
public:
    virtual void AfterAcceptSuccess(const std::shared_ptr<PollerEvent>& client, EventLoop *loop, const std::shared_ptr<moxie::NetAddress>& cad) {
        if (!loop->Register(client, std::make_shared<EchoClientHandler>())) {
            ::close(client->GetFd());
            return;
        }
    }
};

struct WorkContext {
    short port;
    std::string ip;
};

void* DoEcho(void* data) {
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
    if (!loop->Register(event, std::make_shared<Echo>())) {
        LOGGER_ERROR("Loop Register Error");
        return nullptr;
    }

    // 进入loop循环，执行epoll_wait
    loop->Loop();

    delete loop;
    return nullptr;
}

#ifndef THREAD_NUM
#define THREAD_NUM 4
#endif

int main (int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage:MultiEcho ip port" << std::endl;
        return -1;
    }

    WorkContext *ctx = new (std::nothrow) WorkContext;
    if (ctx == nullptr) {
        LOGGER_ERROR("New WorkContext failed!");
        return -1;
    }

    ctx->port = std::atoi(argv[1]);
    ctx->ip = argv[2];

    std::vector<pthread_t> thds; thds.reserve(THREAD_NUM);
    for (int i = 0; i < THREAD_NUM; ++i) {
        pthread_t td;
        if (pthread_create(&td, nullptr, DoEcho, ctx) < 0) {
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
