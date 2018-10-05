#include <functional>
#include <sys/eventfd.h>
#include <utility>

#include <EventLoopThread.h>
#include <Timer.h>

moxie::EventLoopThread::EventLoopThread() 
    : loopThread_(new InnerLoopThread) {
    assert(this->loopThread_);
}

void moxie::EventLoopThread::Start() {
    this->loopThread_->Start();
}

void moxie::EventLoopThread::Stop() {
    this->loopThread_->Stop();
}

void moxie::EventLoopThread::Join() {
    this->loopThread_->Join();
}

void moxie::EventLoopThread::PushTask(const std::function<void (EventLoop *loop)>& task) {
    this->loopThread_->Push(task);
}

