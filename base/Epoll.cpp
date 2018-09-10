#include <iostream>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>


#include <Epoll.h>
#include <PollerEvent.h>
#include <Log.h>

moxie::Epoll::Epoll() :
    epoll_fd_(-1),
    revents_(),
    maxNum_(20),
    addStep_(15) {//FIXME : EPOLL_CLOEXEC
    epoll_fd_ = ::epoll_create1(EPOLL_CLOEXEC);
    if(epoll_fd_ == -1) {
        LOGGER_SYSERR("epoll_create error : " << ::strerror(errno));
    }
    revents_.resize(maxNum_);
}

bool moxie::Epoll::EventCtl(int op, int sd, struct epoll_event* event) {
    assert(epoll_fd_ != -1);
    int ret = ::epoll_ctl(epoll_fd_, op, sd, event);
    if (ret == -1) {
        LOGGER_SYSERR("epoll_ctl error : " << ::strerror(errno) << " fd : " << sd);
        return false;
    } else {
        return true;
    }
}

bool moxie::Epoll::EventAdd(int sd, struct epoll_event* event) {
    return EventCtl(EPOLL_CTL_ADD, sd, event);
}

bool moxie::Epoll::EventDel(int sd, struct epoll_event* event) {
    return EventCtl(EPOLL_CTL_DEL, sd, event);
}

bool moxie::Epoll::EventMod(int sd, struct epoll_event* event) {
    return EventCtl(EPOLL_CTL_MOD, sd, event);
}

int moxie::Epoll::LoopWait(struct epoll_event* events, int maxevents, int timeout) {
    //FIXME : The call was interrupted by a signal
should_continue:
    int ret = ::epoll_wait(epoll_fd_, events, maxevents, timeout);
    if (ret == -1) {
        if (errno == EINTR) {
            LOGGER_SYSERR("epoll_wait error : " << ::strerror(errno));
            goto should_continue;
        }
        LOGGER_SYSERR("epoll_wait error : " << ::strerror(errno));
    }
    return ret;
}




