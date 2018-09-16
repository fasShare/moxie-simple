#ifndef MOXIE_TIMER_H
#define MOXIE_TIMER_H
#include <functional>

#include <Timestamp.h>

namespace moxie {

class EventLoop;
class Timer;
using TimerCallback = std::function<void (Timer *, EventLoop *)>; 

#define SHOULDADDED     (1 << 0)
#define INQUEUED        (1 << 1)
#define INRUNNING       (1 << 2)
#define INEXPIRED       (1 << 3)
#define WILLREMOVED     (1 << 4)

class Timer {
public:
    Timer(const TimerCallback& cb, Timestamp when, double interval = 0);
    ~Timer();

    uint32_t State() const;
    void State(uint32_t state);

    void Run(Timer *t, EventLoop *loop) const;
    Timestamp Expiration() const;
    bool Repeat() const;

    void Restart();
    void Reset(Timestamp when);
    void Shutdown();
private:
    const TimerCallback callback_;
    Timestamp expiration_;
    const double interval_;
    const bool repeat_;
    uint32_t state_;
};

}
#endif  // MOXIE_TIMER_H
