#ifndef MOXIE_MUTEX_H
#define MOXIE_MUTEX_H
#include <pthread.h>

namespace moxie {

class Mutex {
public:
    Mutex() {
        mutex_ = PTHREAD_MUTEX_INITIALIZER;
    }
    void lock() {
        ::pthread_mutex_lock(&mutex_);
    }
    void unlock() {
        ::pthread_mutex_unlock(&mutex_);
    }
    pthread_mutex_t* getMutex() {
        return &mutex_;
    }
    ~Mutex() {
        ::pthread_mutex_destroy(&mutex_);
    }
private:
    pthread_mutex_t mutex_;
};

}
#endif // MOXIE_MUTEX_H
