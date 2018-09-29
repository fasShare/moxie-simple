#ifndef MOXIE_PAGE_H
#define MOXIE_PAGE_H
#include <stdlib.h>
#include <stddef.h>

#include "mlist.h"

namespace moxie {
typedef unsigned long long  uint64_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;

const uint64_t PAGE_ALIGN_BYTES  = 1048576; /* 1M */
const uint16_t CHUNK_ALIGN_BYTES = (sizeof(void *));
#define PAGE_FROM_CHUNK(chunk) (((moxie::Page **)ptr - 1)[0])
#define PAGE_FROM_PLIST(pplist) ((Page *)((const char *)pplist - offsetof(moxie::Page, plist)))

class Page {
public:
    friend class Slab;
    Page(size_t page_size, size_t trunk_size);
    ~Page();
    bool reset(size_t chunk_size);
    bool is_empty() const;
    bool vaild() const;
    bool is_full() const;
    void *alloc_chunk();
    bool free_chunk(void *);
public:
    void *ptr;
    moxie_list::mlist plist;
    size_t chunk_size;
    size_t page_size;
    size_t chunk_num;
    void **free_chunk_list;
    size_t free_chunk_end;
};

} // moxie

#endif 