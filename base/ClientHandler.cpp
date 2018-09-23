#include <unistd.h>

#include <ClientHandler.h>
#include <EventLoop.h>
#include <Log.h>

#define TMP_BUF_SIZE 512

moxie::ClientHandler::ClientHandler() :
    readBuf_(TMP_BUF_SIZE),
    writeBuf_(TMP_BUF_SIZE) {
}

void moxie::ClientHandler::Process(const std::shared_ptr<PollerEvent>& event, EventLoop *loop) {
    if (event->ValatileErrorEvent() || event->ValatileCloseEvent()) {
        loop->Delete(event);
        ::close(event->GetFd());
        return ;
    }

    if (event->ValatileWriteEvent()) {
        DoWrite(event, loop);
    }

    if (event->ValatileReadEvent()) {
        DoRead(event, loop);
    }
}

void moxie::ClientHandler::DoRead(const std::shared_ptr<PollerEvent>& event, EventLoop *loop) {
    int event_fd = event->GetFd();
    int errno_tmp;
    while (true) {
        int ret = readBuf_.readFd(event_fd, &errno_tmp);
        if (ret < 0) {
            if (errno_tmp == EINTR) {
                continue;
            } else if (errno_tmp == EAGAIN || errno_tmp == EWOULDBLOCK) {
                goto AfterReadLabel;
            } else {
                loop->Delete(event);
                ::close(event_fd);
                return;
            }
        }

        if (ret == 0) {
            loop->Delete(event);
            ::close(event_fd);
            return;
        }
    }

AfterReadLabel:
    if (readBuf_.readableBytes() > 0) {
        try {
            this->AfetrRead(event, loop);
        } catch (...) {
            LOGGER_WARN("AfetrRead has an exception!");
        }
    }
}

void moxie::ClientHandler::DoWrite(const std::shared_ptr<PollerEvent>& event, EventLoop *loop) {
    LOGGER_TRACE("Begin ClientHandler DoWrite!");
    int event_fd = event->GetFd();
    size_t len = writeBuf_.readableBytes();
    while (len > 0) {
        int ret = write(event_fd, writeBuf_.peek(), len);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                goto AfterWriteLabel;
            } else {
                loop->Delete(event);
                ::close(event_fd);
                return;
            }
        }

        if (ret == 0) {
            loop->Delete(event);
            ::close(event_fd);
            return;
        }

        len -= ret;
        writeBuf_.retrieve(ret);
    }

AfterWriteLabel:
    try {
        this->AfetrWrite(event, loop);
    } catch (...) {
        LOGGER_WARN("AfetrWrite has an exception!");
    }
}
