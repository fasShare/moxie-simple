#ifndef MOXIE_UTILS_BITSOPS_H
#define MOXIE_UTILS_BITSOPS_H

namespace moxie {

namespace utils {

template <typename T>
T SetBits(T val, T bits) {
    return val | bits;
}

template <typename T>
T ClearBits(T val, T bits) {
    return val & (~bits);
}

template <typename T>
bool ContainsAllBits(T val, T bits) {
    return (val & bits) == bits;
}

template <typename T>
bool ContainsOneOfBits(T val, T bits) {
    return val & bits;
}

}

}

#endif