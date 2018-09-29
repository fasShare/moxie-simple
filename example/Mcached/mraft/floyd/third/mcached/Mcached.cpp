#include <mcached/Mcached.h>

moxie::Mcached::Mcached() {
    slots_.reserve(kSlotNum);
    for (size_t index = 0; index < kSlotNum; ++index) {
        slots_.emplace_back(new SlotObject);
    }
}

moxie::Mcached::~Mcached() {
    for (size_t index = 0; index < kSlotNum; ++index) {
        delete slots_[index];
        slots_[index] = nullptr;
    }
}

int moxie::Mcached::ExecuteCommand(const std::vector<std::string>& args, std::string& response) {
    return InnerExecuteCommand(args, response);
}

int moxie::Mcached::ExecuteCommand(const::google::protobuf::RepeatedPtrField<::std::string>& args, std::string& response) {
    return InnerExecuteCommand(args, response);
}

bool moxie::Mcached::IsReadOnly(const std::string& cmdname){
    if (cmdname == "get" || cmdname == "GET") {
        return true;
    }
    return false;
}

size_t moxie::Mcached::BKDRHash(const char *str) {  
    register size_t hash = 0;  
    while (size_t ch = (size_t)*str++) {         
        hash = hash * 131 + ch; 
    }  
    return hash;  
} 
