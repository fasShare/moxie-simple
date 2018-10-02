#include <string>

#include <StringOps.h>

std::vector<std::string> moxie::utils::StringSplit(const std::string& str, const std::string& d) {
    std::string::size_type pos = 0;
    std::vector<std::string> ret;
    while (pos < str.size() && pos != std::string::npos) {
        std::string::size_type start = str.find_first_not_of(d, pos);
        if (start == std::string::npos) {
            break;
        }
        pos = start;
        std::string::size_type end = str.find_first_of(d, pos);
        if (end == std::string::npos) {
            end = str.size();
        }
        pos = end;

        ret.push_back(str.substr(start, end - start));
    }

    return ret;
}

std::string moxie::utils::StringTrim(const std::string& str) {
    if (str.empty()) {
        return str;
    }

    size_t start = str.find_first_not_of(" ");
    size_t end = str.find_last_not_of(" ");
    return str.substr(start, end - start + 1);
}