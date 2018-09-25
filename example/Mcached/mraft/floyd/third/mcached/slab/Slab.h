#ifndef MOXIE_SLAB_H
#define MOXIE_SLAB_H
#include <stdlib.h>

namespace moxie {
typedef unsigned long long  uint64_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
const uint64_t PAGE_ALIGN_BYTES  = 1048576; /* 1M */
const uint16_t CHUNK_ALIGN_BYTES = (sizeof(void *));
const uint32_t SLAB_MAGIC_NUMBER = 0xABCDEFAA;

class Slab {
public:
    friend class MemCache;
    /* create a slab structure and initialize it */
    Slab(size_t chunk_size, size_t page_size, int pre_alloc);
    /* Allocate object of given length. 0 on error */
    void *slab_alloc_chunk();
    /* Free previously allocated object */
    bool slab_free_chunk(void *ptr);
private:
    /* new page */
    bool slab_new_page();
private:
    size_t chunk_size;      /* sizes of chunk */
    size_t page_size;       /* sizes of page */
    u_int32_t chunk_number_per_page;   /* how many chunks per page */

    void *end_page_ptr; /* pointer to next free item at end of page, or 0 */
    size_t end_page_free; /* number of chunks remaining at end of last alloced page */
    size_t page_total;  /* how many page were allocated for this class */
    
    void **free_chunk_list;     /* list of chunk ptrs */
    size_t free_chunk_list_length;  /* size of previous array */
    size_t free_chunk_end;   /* first free chunk */
    u_int32_t magic_number;    
};

}

#define GET_SLAB_FROM_CHUNK(ptr) (((Slab *)ptr) - 1)
#define CHECK_SLAB_MAGIC_NUMBER(ptr) (SLAB_MAGIC_NUMBER == ptr->magic_number)

#endif
