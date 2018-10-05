#ifndef MOXIE_THREAD_H
#define MOXIE_THREAD_H
#include <pthread.h>
#include <string>
#include <syscall.h>
#include <functional>

namespace moxie {

class Thread {
public:
    Thread();
    Thread(const std::string& name);
    Thread(std::function<void ()> threadFunc);
    Thread(std::function<void ()> threadFunc, const std::string name);

    bool setThreadFunc(std::function<void ()> threadFunc);
    bool Join();
    bool Stop();
    bool MainThread();
    bool Start();
    std::string getName();
    void setName(const std::string& name);
private:
    friend void* run(void *);
    void ThreadFunc();
    pthread_t threadId_;
    std::string name_;
    std::function<void ()> threadFunc_;
};

void* run(void *);
#define gettid() (::syscall(SYS_gettid))

}
#endif //MOXIE_THREAD_H
