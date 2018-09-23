#ifndef MOXIE_MCACHED
#define MOXIE_MCACHED
#include <vector>
#include <string>
#include <memory>

#include <mcached/SlotObject.h>
#include <mcached/Mutex.h>
#include <google/protobuf/repeated_field.h>

namespace moxie {

class Mcached {
public:
    Mcached();
    int ExecuteCommand(const std::vector<std::string>& arg, std::string& response);
    int ExecuteCommand(const::google::protobuf::RepeatedPtrField<::std::string>& args, std::string& response);
    static bool IsReadOnly(const std::string& cmdname);
private:
    std::vector<std::shared_ptr<SlotObject>> slots;
    moxie::Mutex mutex_;
};

}

#endif
