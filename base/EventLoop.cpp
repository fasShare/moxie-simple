#include <Log.h>
#include <EventLoop.h>

size_t moxie::EventLoop::kEpollRetBufSize = 128;
size_t moxie::EventLoop::kDefaultTimeOut = 1000;

moxie::EventLoop::EventLoop() :
    epoll_(new (std::nothrow) Epoll()),
    quit_(false),
    started_(false) {
    if (!epoll_) {
        assert(false);
    }

    te_ = std::make_shared<TimerEngine>();
    assert(te_);

    std::shared_ptr<PollerEvent> event = std::make_shared<PollerEvent>(te_->TimerFd(), moxie::kReadEvent);
    if (!this->Register(event, te_)) {
        LOGGER_FATAL("Register Timer event failed!");
    }
}

bool moxie::EventLoop::Register(const std::shared_ptr<PollerEvent>& event, const std::shared_ptr<Handler>& handler) {
    assert(event);
    int event_fd = event->GetFd();
    if (contexts_.count(event_fd) > 0) {
        return false;
    }

    EventContext *context = new (std::nothrow) EventContext();
    if (!context) {
        return false;
    }
    context->handle = handler;
    context->fd = event_fd;
    context->event = event;

    struct epoll_event et;
    et.events = event->GetEvents();
    et.data.ptr = context;

    contexts_[event_fd] = context;

    if (!epoll_->EventAdd(event_fd, &et)) {
        contexts_.erase(event_fd);
        delete context;
        return false;
    }

    return true;
}

bool moxie::EventLoop::Modity(const std::shared_ptr<PollerEvent>& event) {
    assert(event);
    int event_fd = event->GetFd();
    if (contexts_.count(event_fd) != 1) {
        return false;
    }

    struct epoll_event et;
    et.events = event->GetEvents();
    et.data.ptr = contexts_[event_fd];

    return epoll_->EventMod(event_fd, &et);
}

bool moxie::EventLoop::Delete(const std::shared_ptr<PollerEvent>& event) {
    assert(event);
    int event_fd = event->GetFd();
    if (contexts_.count(event_fd) != 1) {
        return false;
    }

    if(!epoll_->EventDel(event_fd, nullptr)) {
        return false;
    }

    EventContext *context = contexts_[event_fd];
    assert(contexts_.erase(event_fd) == 1);
    delete context;
    ::close(event_fd);
    return true;
}

bool moxie::EventLoop::RegisterTimer(Timer* timer) {
    assert(te_);
    return te_->RegisterTimer(timer);
}

bool moxie::EventLoop::UnregisterTimer(Timer* timer) {
    assert(te_);
    return te_->UnregisterTimer(timer);
}

void moxie::EventLoop::Loop() {
    std::vector<struct epoll_event> epoll_ret_buf;
    epoll_ret_buf.resize(kEpollRetBufSize);
    started_ = true;
    while (!quit_) {
        int ret = epoll_->LoopWait(epoll_ret_buf.data(), kEpollRetBufSize, kDefaultTimeOut);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                LOGGER_FATAL("Epoll Wait:" << strerror(errno));
                return;
            }
        }
        // reset cur, otherwise the event was delete in process will longer survival time.
        std::shared_ptr<PollerEvent> cur = nullptr;
        EventContext *pcontext = nullptr;
        struct epoll_event *pet = nullptr;

        for (int index = 0; index < ret; ++index) {
            pet = epoll_ret_buf.data() + index;
            assert(pet->data.ptr);
            pcontext = static_cast<EventContext *>(pet->data.ptr);
            assert(pcontext);
            assert(pcontext->handle);
            assert(pcontext->event);

            cur = pcontext->event;
            cur->SetValatileEvents(pet->events);

            try {
                pcontext->handle->Process(cur, this);
            } catch (...) {
                LOGGER_WARN("Handle process has an Exception!");
            }
        }
    }
}

void moxie::EventLoop::Quit() {
    quit_ = true;
}

bool moxie::EventLoop::Started() const {
    return started_;
}
