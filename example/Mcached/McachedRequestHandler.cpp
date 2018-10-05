#include <McachedRequestHandler.h>
#include <mraft/floyd/include/floyd.h>

extern floyd::Floyd *floyd_raft;

moxie::McachedClientHandler::McachedClientHandler(const std::shared_ptr<PollerEvent>& client,  const std::shared_ptr<moxie::NetAddress>& cad) :
    event_(client),
    peer_(cad),
    argc_(0),
    argv_() {
}

void moxie::McachedClientHandler::AfetrRead(const std::shared_ptr<PollerEvent>& event, EventLoop *loop) {
    if (ParseRedisRequest()) {
        DoMcachedCammand();
    } 
    
    if (writeBuf_.writableBytes()) {
        event_->EnableWrite();
        loop->Modity(event);
    }
}

void moxie::McachedClientHandler::AfetrWrite(const std::shared_ptr<PollerEvent>& event, EventLoop *loop) {
    if (writeBuf_.readableBytes() > 0) {
        return;
    }
    event->DisableWrite();
    loop->Modity(event);
}

bool moxie::McachedClientHandler::DoMcachedCammand() {
    ResetArgcArgv reset(argc_, argv_);
    assert(argc_ == argv_.size());
    assert(argc_ > 0);
    assert(floyd_raft);

    slash::Status s;
    std::string res;

    s = floyd_raft->ExecMcached(argv_, res);

    if (s.ok()) {
        if (argv_[0] == "SET" || argv_[0] == "set") {
            res = "+OK\r\n";
            ReplyString(res);
        } else if (argv_[0] == "get" || argv_[0] == "GET") {
            ReplyBulkString(res);
        }
    } else {
        if (argv_[0] == "SET" || argv_[0] == "set") {
            res = "-ERR write " + s.ToString() + " \r\n";
        } else if (argv_[0] == "get" || argv_[0] == "GET") {
            res = "-ERR read " + s.ToString() + " \r\n";
        } else {
            res = "-ERR request " + s.ToString() + " \r\n";
        }
        ReplyString(res);
    }
    
    return true;
}

bool moxie::McachedClientHandler::ParseRedisRequest() {
    if (readBuf_.readableBytes() <= 0) {
        return false;
    }

    if (argc_ <= 0) { // parse buffer to get argc
        argv_.clear();
        curArgvlen_ = -1;

        const char* crlf = readBuf_.findChars(Buffer::kCRLF, 2);
        if (crlf == nullptr) {
            return false;
        }
        std::string argc_str = readBuf_.retrieveAsString(crlf - readBuf_.peek());
        if (argc_str[0] != '*') {
            ReplyString("-ERR Protocol error: invalid multibulk length\r\n");
            return false;
        }

        argc_ = std::atoi(argc_str.c_str() + sizeof('*'));
        if (argc_ == 0) {
            ReplyString("-ERR Protocol error: invalid multibulk length\r\n");
            return false;
        }
        // \r\n
        readBuf_.retrieve(2);
    }
    
    while (argv_.size() < argc_) {
        const char* crlf = readBuf_.findChars(Buffer::kCRLF, 2);
        if (crlf == nullptr) {
            return false;
        }
        
        if (curArgvlen_ < 0) {
            std::string argv_len = readBuf_.retrieveAsString(crlf - readBuf_.peek());
            if (argv_len[0] != '$') {
                curArgvlen_ = -1;
                argc_ = 0;
                ReplyString("-ERR Protocol error: invalid bulk length (argv_len)\r\n");
                return false;
            }
            // remove $
            curArgvlen_ = std::atoi(argv_len.c_str() + sizeof('$'));
            readBuf_.retrieve(2);
        } else {
            std::string argv_str = readBuf_.retrieveAsString(crlf - readBuf_.peek());
            if (static_cast<ssize_t>(argv_str.size()) != curArgvlen_) {
                curArgvlen_ = -1;
                argc_ = 0;
                ReplyString("-ERR Protocol error: invalid bulk length (argv_str)\r\n");
                return false;
            }
            argv_.push_back(argv_str);
            // \r\n
            readBuf_.retrieve(2);
            curArgvlen_ = -1; // must do, to read next argv item
        }
    }
    // request can be solved
    if ((argc_ > 0) && (argv_.size() == argc_)) {
        return true;
    }

    return false;
}

void moxie::McachedClientHandler::DebugArgcArgv() const {
    std::cout << peer_->getIp() << ":" << peer_->getPort() << "->";
    for (size_t index = 0; index < argc_; index++) {
        std::cout << argv_[index];
        if (index != argc_ - 1) {
            std::cout << " ";
        }
    }
    std::cout << std::endl;
}

void moxie::McachedClientHandler::ReplyString(const std::string& error) {
    writeBuf_.append(error.c_str(), error.size());
}

void moxie::McachedClientHandler::ReplyBulkString(const std::string& item) {
    writeBuf_.append("$", 1);
    std::string len = std::to_string(item.size());
    writeBuf_.append(len.c_str(), len.size());
    writeBuf_.append(Buffer::kCRLF, 2);

    writeBuf_.append(item.c_str(), item.size());
    writeBuf_.append(Buffer::kCRLF, 2);
}