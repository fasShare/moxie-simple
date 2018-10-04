#include <vector>
#include <utility>
#include <algorithm>

#include <TimerEngine.h>
#include <Epoll.h>
#include <EventLoop.h>
#include <Timestamp.h>
#include <Log.h>

moxie::TimerEngine::TimerEngine() :
    expired_(),
    td_(::timerfd_create(CLOCK_MONOTONIC, 0)) {
}

struct itimerspec moxie::TimerEngine::CalcNewTimeVal(moxie::Timestamp earlist) {
    int64_t microseconds =  earlist.GetMicroSecondsSinceEpoch() - \
                            moxie::Timestamp::Now().GetMicroSecondsSinceEpoch();
    if (microseconds < 100) {
        microseconds = 100;
    }

    struct itimerspec newvalue;
    newvalue.it_interval.tv_sec = 0;
    newvalue.it_interval.tv_nsec = 0;

    newvalue.it_value.tv_sec = static_cast<time_t>(microseconds / moxie::Timestamp::kMicroSecondsPerSecond);
    newvalue.it_value.tv_nsec = static_cast<long>((microseconds % moxie::Timestamp::kMicroSecondsPerSecond) * 1000);

    return newvalue;
}

bool moxie::TimerEngine::ResetTd(moxie::Timestamp earlist) {
    struct itimerspec newvalue;
    struct itimerspec oldvalue;
    bzero(&newvalue, sizeof(struct itimerspec));
    bzero(&oldvalue, sizeof(struct itimerspec));
    newvalue = CalcNewTimeVal(earlist);
    int ret = ::timerfd_settime(td_, 0, &newvalue, &oldvalue);
    if (ret < 0) {
        LOGGER_SYSERR("timerfd_settime error : " << ::strerror(errno));
        return false;
    }
    return true;
}

moxie::Timestamp moxie::TimerEngine::EarlistExpiration() const {
    if (timers_.size() == 0) {
        return moxie::AddTime(Timestamp::Now(), 100);
    } else {
        return timers_.begin()->first;
    }
}

void moxie::TimerEngine::ExpiredTimers(std::list<std::pair<moxie::Timestamp,
        moxie::Timer*>>& expired,
        moxie::Timestamp now) {
    auto sentry(std::make_pair(now, reinterpret_cast<moxie::Timer*>(UINTPTR_MAX)));
    auto end = timers_.lower_bound(sentry);
    assert(end == timers_.end() || now < end->first);
    for (auto iter = timers_.begin(); iter != end; ++iter) {
        if (iter->second->State() == INQUEUED) {
            iter->second->State(INEXPIRED);
        }
        expired.push_back(*iter);
    }
    timers_.erase(timers_.begin(), end);
}

int moxie::TimerEngine::TimerFd() const {
    return this->td_;
}

void moxie::TimerEngine::RestartTimer(std::list<std::pair<moxie::Timestamp, moxie::Timer*>>& expired) {
    for (auto iter = expired.begin(); iter != expired.end(); ++iter) {
        if (iter->second->State() & WILLREMOVED) {
            timerExist_.erase(iter->second);
            delete iter->second;
            iter->second = nullptr;
            continue;
        }

        iter->second->State(iter->second->State() & (~INEXPIRED));

        if (iter->second->Repeat()) {
            iter->second->Restart();
            RegisterTimer(iter->second);
            continue;
        } else if (iter->second->State() & SHOULDADDED) {
            RegisterTimer(iter->second);
        } else {
            timerExist_.erase(iter->second);
            delete iter->second;
            iter->second = nullptr;
        }
    }
}

bool moxie::TimerEngine::RegisterTimer(moxie::Timer *timer) {
    if (!timer || !timer->Expiration().Isvalid()) {
        return false;
    }

    if (timer->State() & WILLREMOVED) {
        return false;
    }

    if (timer->State() & INRUNNING) {
        timer->State(timer->State() | SHOULDADDED);
        return true;
    }

    if (timer->State() & INEXPIRED || timer->State() & INQUEUED) {
        return false;
    }

    if (timerExist_.count(timer) == 1) {
        auto key = std::pair<Timestamp, Timer *>(timerExist_[timer], timer);
        if (1 == timers_.count(key)) {
            timers_.erase(key);
        }
    }

    if (timers_.count(std::pair<Timestamp, Timer *>(timer->Expiration(),timer)) > 0) {
        return false;
    }

    timers_.insert(std::pair<Timestamp, Timer *>(timer->Expiration(),timer));
    timerExist_[timer] = timer->Expiration();
    timer->State(INQUEUED);

    Timestamp earlist = EarlistExpiration();
    return ResetTd(earlist);
}

bool  moxie::TimerEngine::UnregisterTimer(moxie::Timer *timer) {
    if (!timer || timerExist_.count(timer) == 0) {
        return false;
    }

    // The timer is running.
    if (timer->State() & INRUNNING || timer->State() & INEXPIRED) {
        timer->State(timer->State() | WILLREMOVED);
        return true;
    }

    assert(timer->State() & INQUEUED);
    timers_.erase(std::pair<Timestamp, Timer *>(timerExist_[timer], timer));

    timerExist_.erase(timer);

    delete timer;
    timer = nullptr;

    Timestamp earlist = EarlistExpiration();
    ResetTd(earlist);
    return true;
}

void moxie::TimerEngine::Process(const std::shared_ptr<PollerEvent>& event, EventLoop *loop) {
    assert(event->GetFd() == TimerFd());
    // now is more accurate than the time of loop wait returned.
    Timestamp now = Timestamp::Now();
    ExpiredTimers(expired_, now);

    for (auto iter = expired_.begin(); iter != expired_.end(); ++iter) {
        assert(iter->second);
        if (iter->second->State() == INEXPIRED) {
            iter->second->State(iter->second->State() | INRUNNING);
            iter->second->Run(iter->second, loop);
            iter->second->State(iter->second->State() & (~INRUNNING));
        }
    }

    RestartTimer(expired_);
    expired_.clear();
    ResetTd(EarlistExpiration());
}

moxie::TimerEngine::~TimerEngine() {
    for (auto iter = expired_.begin(); iter != expired_.end(); ++iter) {
        if (iter->second != nullptr) {
            delete iter->second;
            iter->second = nullptr;
        }
    }

    std::vector<std::pair<moxie::Timestamp, moxie::Timer *>> destroy;
    std::copy(timers_.begin(), timers_.end(), std::back_inserter(destroy));
    timers_.clear();
    for (auto iter = destroy.begin(); iter != destroy.end(); ++iter) {
        if (iter->second != nullptr) {
            delete iter->second;
            iter->second = nullptr;
        }
    }
}
