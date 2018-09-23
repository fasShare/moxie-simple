#include <errno.h>
#include <sys/uio.h>
#include <sys/types.h>

#include <Buffer.h>
#include <Socket.h>

const char moxie::Buffer::kCRLF[] = "\r\n";
const size_t moxie::Buffer::kCheapPrepend;

moxie::Buffer::Buffer(size_t initialSize) :
    initialSize_(initialSize),
    buffer_(kCheapPrepend + initialSize_),
    readerIndex_(kCheapPrepend),
    writerIndex_(kCheapPrepend) {
    assert(readableBytes() == 0);
    assert(writableBytes() == initialSize_);
    assert(prependableBytes() == kCheapPrepend);
}

void moxie::Buffer::swap(Buffer& rhs) {
    buffer_.swap(rhs.buffer_);
    std::swap(readerIndex_, rhs.readerIndex_);
    std::swap(writerIndex_, rhs.writerIndex_);
}

size_t moxie::Buffer::prependableBytes() const {
    return readerIndex_;
}

size_t moxie::Buffer::readableBytes() const {
    return writerIndex_ - readerIndex_;
}

size_t moxie::Buffer::writableBytes() const {
    return buffer_.size() - writerIndex_;
}

const char* moxie::Buffer::peek() const {
    return begin() + readerIndex_;
}

void moxie::Buffer::retrieve(size_t len) {
    assert(len <= readableBytes());
    if (len < readableBytes()) {
        readerIndex_ += len;
    }
    else {
        retrieveAll();
    }
}

void moxie::Buffer::retrieveUntil(const char* end) {
    assert(peek() <= end);
    assert(end <= beginWrite());
    retrieve(end - peek());
}

void moxie::Buffer::retrieveAll() {
    readerIndex_ = kCheapPrepend;
    writerIndex_ = kCheapPrepend;
}

string moxie::Buffer::retrieveAllAsString() {
    return retrieveAsString(readableBytes());;
}

string moxie::Buffer::retrieveAsString(size_t len) {
    assert(len <= readableBytes());
    string result(peek(), len);
    retrieve(len);
    return result;
}

void moxie::Buffer::append(const char* /*restrict*/ data, size_t len) {
    ensureWritableBytes(len);
    std::copy(data, data+len, beginWrite());
    hasWritten(len);
}

void moxie::Buffer::append(const void* /*restrict*/ data, size_t len) {
    append(static_cast<const char*>(data), len);
}

void moxie::Buffer::ensureWritableBytes(size_t len) {
    if (writableBytes() < len) {
        makeSpace(len);
    }
    assert(writableBytes() >= len);
}

char* moxie::Buffer::beginWrite() {
    return begin() + writerIndex_;
}

const char* moxie::Buffer::beginWrite() const {
    return begin() + writerIndex_;
}

char* moxie::Buffer::begin() {
    return &*buffer_.begin();
}

const char* moxie::Buffer::begin() const {
    return &*buffer_.begin();
}

void moxie::Buffer::hasWritten(size_t len) {
    writerIndex_ += len;
}

void moxie::Buffer::makeSpace(size_t len) {    //
    if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
        // FIXME: move readable data
        buffer_.resize(writerIndex_+len);
    } else {
        // move readable data to the front, make space inside buffer
        assert(kCheapPrepend < readerIndex_);
        size_t readable = readableBytes();
        std::copy(begin()+readerIndex_,
                begin()+writerIndex_,
                begin()+kCheapPrepend);
        readerIndex_ = kCheapPrepend;
        writerIndex_ = readerIndex_ + readable;
        assert(readable == readableBytes());
    }
}

const char* moxie::Buffer::findChars(const char* chars, size_t chars_len) const {
    const char* pos = std::search(peek(), beginWrite(), chars, chars+chars_len);
    return pos == beginWrite() ? NULL : pos;
}

ssize_t moxie::Buffer::readFd(int fd, int* savedErrno) {
    // saved an ioctl()/FIONREAD call to tell how much to read
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = begin()+writerIndex_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;
    const ssize_t n = ::readv(fd, vec, 2);
    if (n < 0) {
        *savedErrno = errno;
    } else if (static_cast<size_t>(n) <= writable) {
        writerIndex_ += n;
    } else {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable);
    }

    return n;
}



