#ifndef MOXIE_SLOTOBJECT
#define MOXIE_SLOTOBJECT
#include <unordered_map>
#include <string>

#include <mcached/Mutex.h>
#include <google/protobuf/repeated_field.h>

namespace moxie {

const int32_t kExecOk = 0;
const int32_t kUnKnowError = -1;
const int32_t kNotFoundKey = -2;
const int32_t kExecArgsError = -3;
const int32_t kExecUnknowCmd = -4;

class SlotObject {
public:
    int ApplySet(const std::string& key, const std::string& value, std::string& res);
    int ApplyGet(const std::string& key, std::string& res);
    int applyReplace(const std::string& key, const std::string& value, std::string& res);
private:
    std::unordered_map<std::string, std::string> db_;
    moxie::Mutex mutex_;
};

}

#endif
