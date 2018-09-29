#ifndef MOXIE_BUFFER_H
#define MOXIE_BUFFER_H
#include <algorithm>
#include <vector>
#include <assert.h>
#include <string.h>
#include <string>

using std::string;

namespace moxie {


/*!
 * A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
 * @code
 *  +-------------------+------------------+------------------+
 *  | prependable bytes |  readable bytes  |  writable bytes  |
 *  |                   |     (CONTENT)    |                  |
 *  +-------------------+------------------+------------------+
 *  |                   |                  |                  |
 *  0      <=      readerIndex   <=   writerIndex    <=     size
 *  @endcode
 */
class Buffer {
public:
    static const size_t kCheapPrepend = 8;

    Buffer(size_t initialSize_);
    void swap(Buffer& rhs);
    size_t readableBytes() const;
    size_t writableBytes() const;
    size_t prependableBytes() const;
    const char* peek() const;
    void retrieve(size_t len);
    void retrieveUntil(const char* end);
    void retrieveAll();
    string retrieveAllAsString();
    string retrieveAsString(size_t len);
    void append(const char* data, size_t len);
    void append(const void* data, size_t len);
    void ensureWritableBytes(size_t len);
    char* beginWrite();
    const char* beginWrite() const;
    void hasWritten(size_t len);
    const char* findChars(const char* chars, size_t chars_len) const;

    ssize_t readFd(int fd, int* savedErrno);
public:
    static const char kCRLF[];
private:
    char* begin();
    const char* begin() const;
    void makeSpace(size_t len);
private:
    size_t initialSize_;
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};

}
#endif  // MOXIE_BUFFER_H


