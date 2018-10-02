#include <HttpRequest.h>

moxie::HttpRequest::HttpRequest() 
    : body_(1024) {
    cmd_.clear();
    path_.clear();
    version_.clear();
    headers_.clear();
    keepalive_ = false;
    state_ = STATE_HTTPREQUEST_BEGIN;
}

void moxie::HttpRequest::Init() {
    cmd_.clear();
    path_.clear();
    version_.clear();
    headers_.clear();
    keepalive_ = false;
    state_ = STATE_HTTPREQUEST_BEGIN;
    body_.retrieveAll();
}

bool moxie::HttpRequest::KeepAlive() const {
    return this->keepalive_;
}
void moxie::HttpRequest::SetKeepAlive(bool alive) {
    this->keepalive_ = alive;
}

uint16_t moxie::HttpRequest::GetState() const {
    return this->state_;
}

void moxie::HttpRequest::SetState(uint16_t state) {
    this->state_ = state;
}

std::string moxie::HttpRequest::GetCmd() const {
    return this->cmd_;
}
void moxie::HttpRequest::SetCmd(const std::string& cmd) {
    this->cmd_ = cmd;
}
std::string moxie::HttpRequest::GetPath() const {
    return this->path_;
}
void moxie::HttpRequest::SetPath(const std::string& path) {
    this->path_ = path;
}
std::string moxie::HttpRequest::GetVersion() const {
    return this->version_;
}
void moxie::HttpRequest::SetVersion(const std::string& ver) {
    this->version_ = ver;
}
void moxie::HttpRequest::AddHeaderItem(const std::string& key, const std::string& value) {
    this->headers_[key] = value;
}
std::string moxie::HttpRequest::GetHeaderItem(const std::string& key) const {
    auto iter = this->headers_.find(key);
    if (iter != this->headers_.end()) {
        return iter->second;
    }
    return "";
}

void moxie::HttpRequest::AppendBody(const char *data, size_t length) {
    this->body_.append(data, length);
}
size_t moxie::HttpRequest::GetBodyLength() const {
    return this->body_.readableBytes();
}
const char* moxie::HttpRequest::GetBodyData() const {
    return this->body_.peek();
}