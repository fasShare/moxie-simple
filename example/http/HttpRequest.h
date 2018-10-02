#ifndef MOXIE_HTTPREQUEST_H
#define MOXIE_HTTPREQUEST_H
#include <string>
#include <map>

namespace moxie {

class HttpRequest {
public:
    void Init();
    bool KeepAlive() const;
    void SetKeepAlive(bool alive);
    std::string GetCmd() const;
    void SetCmd(const std::string& cmd);
    std::string GetPath() const;
    void SetPath(const std::string& path);
    std::string GetVersion() const;
    void SetVersion(const std::string& ver);
    void AddHeaderItem(const std::string& key, const std::string& value);
    std::string GetHeaderItem(const std::string& key) const;
private:
    std::string cmd_;
    std::string path_;
    std::string version_;
    bool keepalive_;
    std::map<std::string, std::string> headers_;
};

}

#endif 