#include <HttpRequestHandler.h>
#include <utils/BitsOps.h>
#include <utils/StringOps.h>

moxie::HttpClientHandler::HttpClientHandler(const std::shared_ptr<PollerEvent>& client,  const std::shared_ptr<moxie::NetAddress>& cad) :
    event_(client),
    peer_(cad),
    state_(STATE_HTTPREQUEST_BEGIN) {
}

void moxie::HttpClientHandler::AfetrRead(const std::shared_ptr<PollerEvent>& event, EventLoop *loop) {
    //std::cout << readBuf_.retrieveAllAsString() << std::endl;
    //return;
    
    if (ParseHttpHeader()) {
        // do sth
        ReplyErrorRequest("Not Found!");
    } 

    if (writeBuf_.writableBytes()) {
        event_->EnableWrite();
        loop->Modity(event);
    }
}

void moxie::HttpClientHandler::AfetrWrite(const std::shared_ptr<PollerEvent>& event, EventLoop *loop) {
    if (writeBuf_.readableBytes() > 0) {
        return;
    }

    this->state_ = STATE_HTTPREQUEST_BEGIN;
    request_.Init();
    
    if (!request_.KeepAlive()) {
        loop->Delete(event);
    } else {
        event->DisableWrite();
        loop->Modity(event);
    }
}

bool moxie::HttpClientHandler::ParseHttpHeader() {
    if (readBuf_.readableBytes() <= 0) {
        return false;
    }

    if (this->state_ == STATE_HTTPREQUEST_BEGIN) {
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
            this->state_ = STATE_HTTPREQUEST_BEGIN;
            return false;
        }
        std::cout << fl[0] << " " << fl[1] << " " << fl[2] << std::endl;
        request_.SetCmd(fl[0]);
        request_.SetPath(fl[1]);
        request_.SetVersion(fl[2]);
        // \r\n
        readBuf_.retrieve(2);
        this->state_ = STATE_HTTPREQUEST_FIRSTLINE;
    }

    if (this->state_ == STATE_HTTPREQUEST_FIRSTLINE) {
        const char *end = readBuf_.findChars("\r\n\r\n", 4);
        if (nullptr == end) {
            return false;
        }
        size_t headlen = end - readBuf_.peek();
        std::vector<std::string> fl = moxie::utils::StringSplit(readBuf_.retrieveAsString(headlen), "\r\n");
        for (size_t i = 0; i < fl.size(); ++i) {
            size_t pos = fl[i].find_first_of(":");
            if (pos == std::string::npos) {
                this->state_ = STATE_HTTPREQUEST_BEGIN;
                ReplyErrorRequest("Bad Request!");
                return false;
            }
            std::cout << moxie::utils::StringTrim(fl[i].substr(0, pos)) << " " << moxie::utils::StringTrim(fl[i].substr(pos + 1)) << std::endl;
            request_.AddHeaderItem(moxie::utils::StringTrim(fl[i].substr(0, pos)), moxie::utils::StringTrim(fl[i].substr(pos + 1)));
        }
        // \r\n\r\n
        readBuf_.retrieve(4);
        this->state_ = STATE_HTTPREQUEST_BODY;
        if (request_.GetHeaderItem("Connection") != "close") {
            request_.SetKeepAlive(true);
        }
        return true;
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
