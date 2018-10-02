#include <HttpResponse.h>

moxie::HttpResponse::HttpResponse() 
    :body_(1024) {
}

void moxie::HttpResponse::Init() {
    this->body_.retrieveAll();
    this->headers_.clear();
    this->scode_.clear();
    this->status_.clear();
    this->version_.clear();
}

std::string moxie::HttpResponse::GetVersion() const {
    return this->version_;
}
void moxie::HttpResponse::SetVersion(const std::string& version) {
    this->version_ = version;
}
std::string moxie::HttpResponse::GetScode() const {
    return this->scode_;
}
void moxie::HttpResponse::SetScode(const std::string& scode) {
    this->scode_ = scode;
}
std::string moxie::HttpResponse::GetStatus() const {
    return this->status_;
}
void moxie::HttpResponse::SetStatus(const std::string& status) {
    this->status_ = status;
}
void moxie::HttpResponse::PutHeaderItem(const std::string& key, const std::string& value) {
    this->headers_[key] = value;
}

std::string moxie::HttpResponse::GetHeaderItem(const std::string& key) {
    auto iter = this->headers_.find(key);
    if (iter != this->headers_.end()) {
        return iter->second;
    }
    return "";
}

size_t moxie::HttpResponse::GetBodyLength() const {
    return body_.readableBytes();
}

const char* moxie::HttpResponse::GetBodyData() const {
    return body_.peek();
}

void moxie::HttpResponse::AppendBody(const char *data, size_t length) {
    body_.append(data, length);
}

void moxie::HttpResponse::ToBuffer(moxie::Buffer& out) {
    // head line
    out.append(this->version_.c_str(), this->version_.size());
    out.append(" ", 1);
    out.append(this->scode_.c_str(), this->scode_.size());
    out.append(" ", 1);
    out.append(this->status_.c_str(), this->status_.size());
    out.append(moxie::Buffer::kCRLF, 2);

    // headers
    for (auto& iter : this->headers_) {
        out.append(iter.first.c_str(), iter.first.size());
        out.append(":", 1);
        out.append(iter.second.c_str(), iter.second.size());
        out.append(moxie::Buffer::kCRLF, 2);
    }

    // end empty line
    out.append(moxie::Buffer::kCRLF, 2);

    // body
    out.append(this->body_.peek(), this->body_.readableBytes());
}