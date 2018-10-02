#include "Timestamp.h"
#include <sys/time.h>
#include <stdio.h>
#include <assert.h>
#include <Log.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#undef __STDC_FORMAT_MACROS

moxie::Timestamp::Timestamp() :
    microSecondsSinceEpoch_(0) {
}

moxie::Timestamp::Timestamp(int64_t microseconds) :
    microSecondsSinceEpoch_(microseconds) {
}

std::string moxie::Timestamp::ToString() const {
    char buf[32] = {0};
    int64_t seconds = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
    int64_t microseconds = microSecondsSinceEpoch_ % kMicroSecondsPerSecond;
    snprintf(buf, sizeof(buf)-1, "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
    return buf;
}

std::string moxie::Timestamp::ToFormattedString() const {
    char buf[32] = {0};
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
    int microseconds = static_cast<int>(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);

    snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
            tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
            tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
            microseconds);
    return buf;
}

moxie::Timestamp moxie::Timestamp::Now() {
    return moxie::Timestamp(Timestamp::MacroSeconds());
}

uint64_t moxie::Timestamp::NanoSeconds() {
    struct timespec tn;
    clock_gettime(CLOCK_REALTIME, &tn);
    int64_t seconds = tn.tv_sec;
    return seconds * kNanoSecondsPerSecond + tn.tv_nsec;
}

uint64_t moxie::Timestamp::MacroSeconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t seconds = tv.tv_sec;
    return seconds * kMicroSecondsPerSecond + tv.tv_usec;
}

moxie::Timestamp moxie::Timestamp::Invalid() {
    return Timestamp();
}

bool moxie::Timestamp::Isvalid() const {
    return microSecondsSinceEpoch_ > 0;
}

void moxie::Timestamp::Swap(Timestamp& that) {
    std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
}

int64_t moxie::Timestamp::GetMicroSecondsSinceEpoch() const {
    return microSecondsSinceEpoch_;
}

time_t moxie::Timestamp::SecondsSinceEpoch() const {
    return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
}

bool moxie::operator<(Timestamp lhs, Timestamp rhs) {
    return lhs.GetMicroSecondsSinceEpoch() < rhs.GetMicroSecondsSinceEpoch();
}

bool moxie::operator>(Timestamp lhs, Timestamp rhs) {
    return lhs.GetMicroSecondsSinceEpoch() > rhs.GetMicroSecondsSinceEpoch();
}

bool moxie::operator==(Timestamp lhs, Timestamp rhs) {
    return lhs.GetMicroSecondsSinceEpoch() == rhs.GetMicroSecondsSinceEpoch();
}

double moxie::TimeDifference(Timestamp high, Timestamp low) {
    int64_t diff = high.GetMicroSecondsSinceEpoch() - low.GetMicroSecondsSinceEpoch();
    return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

moxie::Timestamp moxie::AddTime(Timestamp timestamp, double seconds) {
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(timestamp.GetMicroSecondsSinceEpoch() + delta);
}
