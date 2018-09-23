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
#include <McachedServer.h>
#include <mraft/floyd/include/floyd.h>

using namespace moxie;
using namespace floyd;

Floyd *floyd_raft;

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

    EventLoop *loop = new EventLoop();
    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) {
        LOGGER_ERROR("socket error : " << strerror(errno));
        return -1;
    }

    Socket::SetExecClose(server);
    Socket::SetNoBlocking(server);

    NetAddress addr(AF_INET, atoi(argv[5]), "127.0.0.1");

    if (!(Socket::Bind(server, addr)
          && Socket::Listen(server, 128))) {
        return -1;
    }

    // 注册监听套接字的处理类
    std::shared_ptr<PollerEvent> event = std::make_shared<PollerEvent>(server, moxie::kReadEvent);
    if (!loop->Register(event, std::make_shared<McachedServer>())) {
        LOGGER_ERROR("Loop Register Error");
        return -1;
    }

    loop->Loop();

    delete loop;
    return 0;
}
