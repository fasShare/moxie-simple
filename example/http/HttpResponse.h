#ifndef MOXIE_HTTPRESPONSE_H
#define MOXIE_HTTPRESPONSE_H
#include <string>
#include <map>
#include <Buffer.h>

namespace moxie {

class HttpResponse {
public:
    HttpResponse();
    void Init();
    std::string GetVersion() const;
    void SetVersion(const std::string& version);
    std::string GetScode() const;
    void SetScode(const std::string& version);
    std::string GetStatus() const;
    void SetStatus(const std::string& version);
    void PutHeaderItem(const std::string& key, const std::string& value);
    std::string GetHeaderItem(const std::string& key);
    size_t GetBodyLength() const;
    const char* GetBodyData() const;
    void AppendBody(const char *data, size_t length);
    void ToBuffer(moxie::Buffer& out);
private:
    std::string version_;
    std::string scode_;
    std::string status_;
    std::map<std::string, std::string> headers_;
    moxie::Buffer body_;
};

}

#endif 