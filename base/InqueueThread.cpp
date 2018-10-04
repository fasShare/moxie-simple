#include <functional>
#include <sys/eventfd.h>
#include <utility>

#include <InqueueThread.h>
#include <Timer.h>

moxie::InqueueThread::InqueueThread() 
    : loop_(new EventLoop),
    thread_(nullptr) {
    thread_ = new Thread(std::bind(&InqueueThread::run, this));
    this->wakeUpFd_ = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    assert(this->wakeUpFd_ > 0);
}

std::shared_ptr<moxie::InqueueThread> moxie::InqueueThread::CreateInqueueThread() {
    std::shared_ptr<moxie::InqueueThread> ptr(new InqueueThread);
    return ptr;
}

moxie::InqueueThread::~InqueueThread() {
    this->loop_->Quit();
    delete this->loop_;
    delete thread_;
}

void moxie::InqueueThread::Start() {
    std::shared_ptr<PollerEvent> wake = std::make_shared<PollerEvent>(this->wakeUpFd_, kReadEvent);
    loop_->Register(wake, shared_from_this());
    this->thread_->start();
}

void moxie::InqueueThread::Stop() {
    this->loop_->Quit();
    this->thread_->join();
}

void moxie::InqueueThread::InqueueTask(const std::function<void ()>& task) {
    InqueueTaskWithDelayTime(task, 0);
}

void moxie::InqueueThread::InqueueTaskWithDelayTime(const std::function<void ()>& task, uint32_t delay_ms) {
    MutexLocker locker(inqueueTeskMutex_);
    tasks_.emplace(std::pair<std::function<void ()>, uint32_t>(task, delay_ms));
    wakeUp();
}

void moxie::InqueueThread::RunInTimer(Timer *t, EventLoop *loop, std::function<void ()> task) {
    if (task) {
        task();
    }
}

void moxie::InqueueThread::Process(const std::shared_ptr<PollerEvent>& event, EventLoop *loop) {
    std::pair<std::function<void ()>, uint32_t> task;
    while (true) {
        {
            MutexLocker locker(inqueueTeskMutex_);
            if (tasks_.empty()) {
                break;
            }
            task = tasks_.front();
            tasks_.pop();
        }
        if (task.second == 0) {
            task.first();
        } else {
            Timer *t = new Timer(std::bind(RunInTimer, std::placeholders::_1, std::placeholders::_2, task.first), 
                            moxie::AddTime(Timestamp::Now(), (double)(task.second)/1000));
            loop_->RegisterTimer(t);
        }
    }
}

void moxie::InqueueThread::wakeUp() {
    uint64_t one = 1;
    ssize_t n = ::write(wakeUpFd_, &one, sizeof one);
    if (n != sizeof one) {
        LOGGER_ERROR("EventLoop::wakeup() writes " << n << " bytes instead of 8");
    }
}

void moxie::InqueueThread::run() {
    this->loop_->Loop();
}