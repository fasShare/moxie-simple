#ifndef MOXIE_HTTPREQUEST_H
#define MOXIE_HTTPREQUEST_H
#include <string>
#include <map>

#include <Buffer.h>

namespace moxie {

const static uint16_t STATE_HTTPREQUEST_BEGIN = (1 << 0);
const static uint16_t STATE_HTTPREQUEST_FIRSTLINE = (1 << 1);
const static uint16_t STATE_HTTPREQUEST_HEADER = (1 << 2);
const static uint16_t STATE_HTTPREQUEST_BODY = (1 << 3);
const static uint16_t STATE_HTTPREQUEST_PROCESS = (1 << 4);

class HttpRequest {
public:
    HttpRequest();
    void Init();
    bool KeepAlive() const;
    void SetKeepAlive(bool alive);
    uint16_t GetState() const;
    void SetState(uint16_t state);
    std::string GetCmd() const;
    void SetCmd(const std::string& cmd);
    std::string GetPath() const;
    void SetPath(const std::string& path);
    std::string GetVersion() const;
    void SetVersion(const std::string& ver);
    void AddHeaderItem(const std::string& key, const std::string& value);
    std::string GetHeaderItem(const std::string& key) const;
    void AppendBody(const char *data, size_t length);
    size_t GetBodyLength() const;
    const char* GetBodyData() const;
private:
    std::string cmd_;
    std::string path_;
    std::string version_;
    bool keepalive_;
    uint16_t state_;
    std::map<std::string, std::string> headers_;
    moxie::Buffer body_;
};

}

#endif 