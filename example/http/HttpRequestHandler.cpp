#include <HttpRequestHandler.h>
#include <utils/BitsOps.h>
#include <utils/StringOps.h>

moxie::HttpClientHandler::HttpClientHandler(const std::shared_ptr<PollerEvent>& client,  const std::shared_ptr<moxie::NetAddress>& cad) :
    event_(client),
    peer_(cad) {
}

void moxie::HttpClientHandler::SetMethods(const std::map<std::string, std::function<void (moxie::HttpRequest&, moxie::HttpResponse&)>>& method) {
    this->method_ = method;
}

void moxie::HttpClientHandler::AfetrRead(const std::shared_ptr<PollerEvent>& event, EventLoop *loop) {
    if (ParseHttpRequest()) {
        // do sth
        if (request_.GetState() == STATE_HTTPREQUEST_PROCESS) {
            try {
                auto iter = method_.find(request_.GetCmd());
                if (iter == method_.end()) {
                    ReplyErrorRequest("Bad Request! \r\n command not found!");
                } else {
                    iter->second(request_, response_);
                }
            } catch (...) {
                LOGGER_ERROR("Http callback throw an exception!");
            }
        }
        request_.SetState(STATE_HTTPREQUEST_BEGIN);
    } 

    response_.ToBuffer(writeBuf_);

    if (writeBuf_.writableBytes()) {
        event_->EnableWrite();
        loop->Modity(event);
    }
}

void moxie::HttpClientHandler::AfetrWrite(const std::shared_ptr<PollerEvent>& event, EventLoop *loop) {
    if (writeBuf_.readableBytes() > 0) {
        return;
    }
    
    if (!request_.KeepAlive()) {
        loop->Delete(event);
    } else {
        event->DisableWrite();
        loop->Modity(event);
    }
}

bool moxie::HttpClientHandler::ParseHttpRequest() {
    if (readBuf_.readableBytes() <= 0) {
        return false;
    }

    if (request_.GetState() == STATE_HTTPREQUEST_BEGIN) {
        request_.Init();
        response_.Init();
        const char *end = readBuf_.findChars(moxie::Buffer::kCRLF, 2);
        if (nullptr == end) {
            return false;
        }
        int32_t linelen = end - readBuf_.peek();
        std::vector<std::string> fl = moxie::utils::StringSplit(readBuf_.retrieveAsString(linelen), " \r\n");
        if (fl.size() != 3) {
            ReplyErrorRequest("Bad Request!");
            request_.SetState(STATE_HTTPREQUEST_BEGIN);
            return false;
        }
        request_.SetCmd(fl[0]);
        request_.SetPath(fl[1]);
        request_.SetVersion(fl[2]);
        // \r\n
        readBuf_.retrieve(2);
        request_.SetState(STATE_HTTPREQUEST_FIRSTLINE);
    }

    if (request_.GetState() == STATE_HTTPREQUEST_FIRSTLINE) {
        const char *end = readBuf_.findChars("\r\n\r\n", 4);
        if (nullptr == end) {
            return false;
        }
        size_t headlen = end - readBuf_.peek();
        std::vector<std::string> fl = moxie::utils::StringSplit(readBuf_.retrieveAsString(headlen), "\r\n");
        for (size_t i = 0; i < fl.size(); ++i) {
            size_t pos = fl[i].find_first_of(":");
            if (pos == std::string::npos) {
                request_.SetState(STATE_HTTPREQUEST_BEGIN);
                ReplyErrorRequest("Bad Request!");
                return false;
            }
            request_.AddHeaderItem(moxie::utils::StringTrim(fl[i].substr(0, pos)), moxie::utils::StringTrim(fl[i].substr(pos + 1)));
        }
        // \r\n\r\n
        readBuf_.retrieve(4);
        request_.SetState(STATE_HTTPREQUEST_BODY);
        if (request_.GetHeaderItem("Connection") != "close") {
            request_.SetKeepAlive(true);
        }
    }

    if (request_.GetState() == STATE_HTTPREQUEST_BODY) {
        if (request_.GetHeaderItem("Content-length") == "") {
            request_.SetState(STATE_HTTPREQUEST_PROCESS);
            return true;
        }
        if (readBuf_.readableBytes() == static_cast<uint32_t>(std::atol(request_.GetHeaderItem("Content-length").c_str()))) {
            request_.AppendBody(readBuf_.peek(), readBuf_.readableBytes());
            request_.SetState(STATE_HTTPREQUEST_PROCESS);
            return true;
        }
    }

    return false;
}

void moxie::HttpClientHandler::ReplyErrorRequest(const string& error) {
    response_.SetScode("400");
    response_.SetStatus("Bad Request");
    response_.SetVersion(request_.GetVersion());
    
    std::string content = "<html><body>" + error + " </body></html>";
    response_.AppendBody(content.c_str(), content.size());

    response_.PutHeaderItem("Content-Type", "text/html");
    response_.PutHeaderItem("Content-Length", std::to_string(content.size()));

    response_.ToBuffer(writeBuf_);
}
