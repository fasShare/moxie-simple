#include <errno.h>

#include <Condition.h>

bool moxie::Condition::waitForSeconds(int seconds) {
    struct timespec abstime;
    ::clock_gettime(CLOCK_REALTIME, &abstime);
    abstime.tv_sec += seconds;
    return ETIMEDOUT == pthread_cond_timedwait(&cond_, mutex_.getMutex(), &abstime);
}




