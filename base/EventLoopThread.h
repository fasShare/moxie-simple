#ifndef MOXIE_EVENTLOOPTHREAD_H
#define MOXIE_EVENTLOOPTHREAD_H
#include <functional>
#include <queue>
#include <memory>
#include <sys/eventfd.h>

#include <EventLoop.h>
#include <Thread.h>
#include <Mutex.h>
#include <MutexLocker.h>
#include <Condition.h>

namespace moxie {

class EventLoopThread {
public:
    EventLoopThread();
    void Start();
    void Stop();
    void Join();
    void PushTask(const std::function<void (EventLoop *loop)>& task);
private:
    class InnerLoopThread : public Handler, public std::enable_shared_from_this<InnerLoopThread> {
    public:
        InnerLoopThread()
        : tasks_(),
         taskMutex_(),
         condMutex_(),
         cond_(condMutex_),
         loop_(nullptr),
         wakeUpFd_(::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)) {
             assert(this->wakeUpFd_ > 0);
             this->thread_ = new Thread(std::bind(&InnerLoopThread::Run, this));
        }
        ~InnerLoopThread() {
            if (this->loop_) {
                this->loop_->Quit();
                delete this->loop_;
            }
            delete thread_;
        }
        void Start() {
            this->thread_->Start();
        }
        void Join() {
            this->thread_->Join();
        }
        void Stop() {
            this->loop_->Quit();
            this->thread_->Stop();
        }
        void Run() {
            this->loop_ = new EventLoop;
            if (!this->loop_) {
                return;
            }
            std::shared_ptr<PollerEvent> wake = std::make_shared<PollerEvent>(this->wakeUpFd_, kReadEvent);
            loop_->Register(wake, shared_from_this());
            loop_->Loop();
        }
        void WakeUp() {
            uint64_t one = 1;
            ssize_t n = ::write(wakeUpFd_, &one, sizeof one);
            if (n != sizeof one) {
                LOGGER_ERROR("EventLoop::wakeup() writes " << n << " bytes instead of 8");
            }
        }
        void ClearWake() {
            uint64_t one = 1;
            ssize_t n = ::read(wakeUpFd_, &one, sizeof one);
            if (n != sizeof one){
                LOGGER_ERROR("EventLoop::handleRead() reads " << n << " bytes instead of 8");
            }
        }
        void Push(const std::function<void (EventLoop *loop)>& task) {
            MutexLocker locker(taskMutex_);
            tasks_.emplace(task);
            WakeUp();
        }
        void Process(const std::shared_ptr<PollerEvent>& event, EventLoop *loop) {
            ClearWake();
            std::function<void (EventLoop *loop)> task;
            while (true) {
                {
                    MutexLocker locker(taskMutex_);
                    if (tasks_.empty()) {
                        break;
                    }
                    task = tasks_.front();
                    tasks_.pop();
                }
                task(loop);
            }
        }
    private:
        std::queue<std::function<void (EventLoop *loop)>> tasks_;
        Mutex taskMutex_;
        Mutex condMutex_;
        Condition cond_;
        EventLoop *loop_;
        Thread *thread_;
        int wakeUpFd_;
    };
private:
    std::shared_ptr<InnerLoopThread> loopThread_;
};

}

#endif 