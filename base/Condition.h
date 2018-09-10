#ifndef MOXIE_CONDITION_H
#define MOXIE_CONDITION_H
#include <pthread.h>

#include <Mutex.h>

namespace moxie {

class Condition {
public:
    Condition(Mutex& mutex) :
        mutex_(mutex) {
        ::pthread_cond_init(&cond_, NULL);
    }
    ~Condition() {
        ::pthread_cond_destroy(&cond_);
    }

    void wait() {
        ::pthread_cond_wait(&cond_, mutex_.getMutex());
    }

    bool waitForSeconds(int seconds);

    void notify() {
        ::pthread_cond_signal(&cond_);
    }
    void notifyAll() {
        ::pthread_cond_broadcast(&cond_);
    }
private:
    Mutex& mutex_;
    pthread_cond_t cond_;
};

}
#endif  // MOXIE_CONDITION_H
