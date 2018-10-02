#ifndef MOXIE_UTILS_STRINGOPS_H
#define MOXIE_UTILS_STRINGOPS_H
#include <vector>
#include <string>

namespace moxie {

namespace utils {
std::vector<std::string> StringSplit(const std::string& str, const std::string& d);
std::string StringTrim(const std::string& str);
}

}

#endif