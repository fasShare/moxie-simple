#ifndef MOXIE_INQUEUETHREAD_H
#define MOXIE_INQUEUETHREAD_H
#include <functional>
#include <queue>
#include <memory>

#include <EventLoop.h>
#include <Thread.h>
#include <Mutex.h>
#include <MutexLocker.h>

namespace moxie {

class InqueueThread : public Handler, public std::enable_shared_from_this<InqueueThread> {
public:
    static std::shared_ptr<InqueueThread> CreateInqueueThread();
    virtual ~InqueueThread();
    void Start();
    void Stop();
    void InqueueTask(const std::function<void ()>& task);
    void InqueueTaskWithDelayTime(const std::function<void ()>& task, uint32_t delay_ms);
    virtual void Process(const std::shared_ptr<PollerEvent>& event, EventLoop *loop) override;
private:
    static void RunInTimer(Timer *t, EventLoop *loop, std::function<void ()> task);
    InqueueThread();
    void run();
    void wakeUp();
private:
    std::queue<std::pair<std::function<void ()>, uint32_t>> tasks_;
    Mutex inqueueTeskMutex_;
    EventLoop *loop_;
    Thread *thread_;
    int wakeUpFd_;
};

}

#endif 