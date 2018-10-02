#include <HttpRequest.h>

void moxie::HttpRequest::Init() {
    cmd_.clear();
    path_.clear();
    version_.clear();
    headers_.clear();
    keepalive_ = false;
}

bool moxie::HttpRequest::KeepAlive() const {
    return this->keepalive_;
}
void moxie::HttpRequest::SetKeepAlive(bool alive) {
    this->keepalive_ = alive;
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