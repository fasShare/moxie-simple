#include <mcached/SlotObject.h>
#include <mcached/MutexLocker.h>

int moxie::SlotObject::ApplyRequest(const std::vector<std::string>& args, std::string& res) {
    moxie::MutexLocker lock(mutex_);
    if (args.size() == 3 && (args[0] == "set" || args[0] == "SET")) {
        db_[args[1]] = args[2];
        res = "+OK\r\n";
    } else if (args.size() == 2 && (args[0] == "get" || args[0] == "GET")) {
        res = db_[args[1]];
    }

    return 0;
}

int moxie::SlotObject::ApplyRequest(const::google::protobuf::RepeatedPtrField<::std::string>& args, std::string& res) {
    moxie::MutexLocker lock(mutex_);
    if (args.size() == 3 && (args[0] == "set" || args[0] == "SET")) {
        db_[args[1]] = args[2];
        res = "+OK\r\n";
    } else if (args.size() == 2 && (args[0] == "get" || args[0] == "GET")) {
        res = db_[args[1]];
    }

    return 0;
}
