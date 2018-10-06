#include <limits>

#include <IdGenerator.h>

moxie::utils::IdGenerator::IdGenerator(int64_t memberid, uint64_t nano) {
    this->prefix_ = memberid << kSuffixLen;
    uint64_t unixMilli = (uint64_t)Timestamp::NanoSeconds() / (1000000);
    this->suffix_ = lowbit(unixMilli, kTsLen) << kCntLen;
}

uint64_t moxie::utils::IdGenerator::Next() {
    moxie::MutexLocker locker(mutex_);
    this->suffix_++;
    return this->prefix_ | lowbit(this->suffix_, kSuffixLen);
}

uint64_t moxie::utils::IdGenerator::lowbit(uint64_t x, uint32_t n) {
    return x & (std::numeric_limits<uint64_t>::max() >> (64 - n));
}
