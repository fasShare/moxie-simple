#ifndef MOXIE_SLOTOBJECT
#define MOXIE_SLOTOBJECT
#include <unordered_map>
#include <string>

#include <mcached/Mutex.h>
#include <google/protobuf/repeated_field.h>

namespace moxie {

class HashFunc {
public:
    size_t operator()(const std::string& str) const {
        return BKDRHash(str.c_str()); 
    }
private:
    size_t BKDRHash(const char *str) const {  
        register size_t hash = 0;  
        while (size_t ch = (size_t)*str++) {         
            hash = hash * 131 + ch; 
        }  
        return hash;  
    } 
};

class SlotObject {
public:
    int ApplyRequest(const std::vector<std::string>& args, std::string& res);
    int ApplyRequest(const::google::protobuf::RepeatedPtrField<::std::string>& args, std::string& res);
private:
    std::unordered_map<std::string, std::string> db_;
    moxie::Mutex mutex_;
};

}

#endif
