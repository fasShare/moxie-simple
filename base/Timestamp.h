#ifndef MOXIE_TIMESTAMP_H
#define MOXIE_TIMESTAMP_H
#include <string>

namespace moxie {

using std::string;

class Timestamp {
public:
    Timestamp();
    explicit Timestamp(int64_t microSecondsSinceEpoch);
    void Swap(Timestamp& that);
    string ToString() const;
    string ToFormattedString() const;
    static Timestamp Invalid();
    bool Isvalid() const;
    // for internal usage.
    int64_t GetMicroSecondsSinceEpoch() const;
    time_t SecondsSinceEpoch() const;
    static Timestamp Now();
    static const int kMicroSecondsPerSecond = 1000 * 1000;
private:
    int64_t microSecondsSinceEpoch_;
};


bool operator<(Timestamp lhs, Timestamp rhs);
bool operator>(Timestamp lhs, Timestamp rhs);
bool operator==(Timestamp lhs, Timestamp rhs);
double TimeDifference(Timestamp high, Timestamp low);
Timestamp AddTime(Timestamp timestamp, double seconds);

}
#endif  // MOXIE_TIMESTAMP_H
