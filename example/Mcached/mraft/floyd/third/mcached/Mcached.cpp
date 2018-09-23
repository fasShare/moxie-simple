#include <mcached/Mcached.h>
#include <mcached/MutexLocker.h>

moxie::Mcached::Mcached() {
    slots.reserve(1);
    slots.emplace_back(std::make_shared<SlotObject>());
}

int moxie::Mcached::ExecuteCommand(const std::vector<std::string>& args, std::string& response) {
    moxie::MutexLocker lock(mutex_);
    return slots[0]->ApplyRequest(args, response);
}

int moxie::Mcached::ExecuteCommand(const::google::protobuf::RepeatedPtrField<::std::string>& args, std::string& response) {
    moxie::MutexLocker lock(mutex_);
    return slots[0]->ApplyRequest(args, response);
}

bool moxie::Mcached::IsReadOnly(const std::string& cmdname){
    if (cmdname == "get" || cmdname == "GET") {
        return true;
    }
    return false;
}
