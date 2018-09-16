#include <assert.h>

#include <Timer.h>
#include <Log.h>

moxie::Timer::Timer(const moxie::TimerCallback& cb, moxie::Timestamp when, double interval) :
    callback_(cb),
    expiration_(when),
    interval_(interval),
    repeat_(interval > 0.0),
    state_(SHOULDADDED) {
}

uint32_t moxie::Timer::State() const {
    return state_;
}

void moxie::Timer::State(uint32_t state) {
    state_ = state;
}

void moxie::Timer::Run(Timer *t, EventLoop *loop) const {
    assert(callback_);
    try {
        callback_(t, loop);
    } catch (...) {
        LOGGER_ERROR("Timer callback throw an exception!");
    }
}

moxie::Timestamp moxie::Timer::Expiration() const {
    return expiration_;
}

bool moxie::Timer::Repeat() const {
    return repeat_;
}

void moxie::Timer::Restart() {
    if (state_ & WILLREMOVED) {
        return;
    }
    if (repeat_) {
        expiration_ = moxie::AddTime(Timestamp::Now(), interval_);
        state_ |= SHOULDADDED;
    } else {
        assert(false);
    }
}

void moxie::Timer::Reset(Timestamp when) {
    if (state_ & WILLREMOVED) {
        return;
    }
    expiration_ = when;
    state_ |= SHOULDADDED;
}

moxie::Timer::~Timer() {
    LOGGER_TRACE("Destroy one Timer!");
}
