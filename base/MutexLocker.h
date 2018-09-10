#ifndef MOXIE_MUTEXLOCKER_H
#define MOXIE_MUTEXLOCKER_H
#include <pthread.h>

#include <Mutex.h>

namespace moxie {

class MutexLocker {
public:
    MutexLocker(Mutex& mutex) :mutex_(mutex){
        mutex_.lock();
    }

    ~MutexLocker() {
        mutex_.unlock();
    }
private:
    Mutex& mutex_;
};

}
#endif // MOXIE_MUTEXLOCKER_H
