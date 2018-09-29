#include "Mutex.h"

moxie::Mutex::Mutex() {
    mutex_ = PTHREAD_MUTEX_INITIALIZER;
}
moxie::Mutex::~Mutex(){
    ::pthread_mutex_destroy(&mutex_);
}

void moxie::Mutex::lock(){
    ::pthread_mutex_lock(&mutex_);
}

void moxie::Mutex::unlock(){
    ::pthread_mutex_unlock(&mutex_);
}

pthread_mutex_t* moxie::Mutex::getMutex() {
    return &mutex_;
}


