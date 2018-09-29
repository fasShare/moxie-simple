#include <MutexLocker.h>
#include <Mutex.h>

moxie::MutexLocker::MutexLocker(moxie::Mutex& mutex):mutex_(mutex){
    mutex_.lock();
}

moxie::MutexLocker::~MutexLocker(){
    mutex_.unlock();
}
