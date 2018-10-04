#include <iostream>

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
            //readBuf_.retrieveAll();
            std::cout << "[" << readBuf_.retrieveAllAsString() << "]" << std::endl;
            //event->EnableWrite();
            //loop->Modity(event);
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

int main() {
    EventLoop *loop = new EventLoop();
    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) {
        LOGGER_ERROR("socket error : " << strerror(errno));
        return -1;
    }

    Socket::SetExecClose(server);
    Socket::SetNoBlocking(server);

    NetAddress addr(AF_INET, 8899, "127.0.0.1");

    if (!(Socket::Bind(server, addr)
          && Socket::Listen(server, 128))) {
        return -1;
    }

    // 注册监听套接字的处理类
    std::shared_ptr<PollerEvent> event = std::make_shared<PollerEvent>(server, moxie::kReadEvent);
    if (!loop->Register(event, std::make_shared<Echo>())) {
        LOGGER_ERROR("Loop Register Error");
        return -1;
    }

    loop->Loop();

    delete loop;
    return 0;
}
