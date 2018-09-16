#ifndef MOXIE_TIMERENGINE_H
#define MOXIE_TIMERENGINE_H
#include <sys/timerfd.h>
#include <memory>
#include <set>
#include <vector>
#include <list>
#include <unordered_map>

#include <Handler.h>
#include <Timestamp.h>
#include <Timer.h>

namespace moxie {

class TimerEngine : virtual public Handler {
public:
    TimerEngine();
    ~TimerEngine();

    bool RegisterTimer(Timer *);
    bool UnregisterTimer(Timer *);

    int TimerFd() const;

    uint TimerCount() const;

    bool EarlistChange() const;
    Timestamp EarlistExpiration() const;
    void ExpiredTimers(std::list<std::pair<Timestamp, Timer *>>& expired, Timestamp now);
    void RestartTimer(std::list<std::pair<Timestamp, Timer *>>& expired);

    virtual void Process(const std::shared_ptr<PollerEvent>& event, EventLoop *loop);
private:
    struct itimerspec CalcNewTimeVal(Timestamp earlist);
    bool ResetTd(Timestamp earlist);

    std::set<std::pair<Timestamp, Timer *>> timers_;
    std::unordered_map<Timer *, Timestamp> timerExist_;
    std::list<std::pair<Timestamp, Timer*>> expired_;
    int td_;
};

}
#endif // MOXIE_TIMERENGINE_H

