/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * Slabs memory allocation, based on powers-of-N. Slabs are up to 1MB in size
 * and are divided into chunks. The chunk sizes start off at the size of the
 * "item" structure plus space for a small key and value. They increase by
 * a multiplier factor from there, up to half the maximum slab size. The last
 * slab size is always 1MB, since that's the maximum item size allowed by the
 * memcached protocol.
 *
 * $Id$
 */
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include "Slab.h"
#include "Items.h"


moxie::Slab::Slab(size_t chunk_size, size_t page_size, int pre_alloc) {
    /* Make sure chunk are always n-byte aligned */
    if (chunk_size % CHUNK_ALIGN_BYTES) {
        chunk_size += CHUNK_ALIGN_BYTES - (chunk_size % CHUNK_ALIGN_BYTES);
    }
    
    /* Make sure page are always n-byte aligned */
    if (page_size % PAGE_ALIGN_BYTES) {
        page_size += PAGE_ALIGN_BYTES - (page_size % PAGE_ALIGN_BYTES);
    }

    this->chunk_size = chunk_size;
    this->page_size = page_size;
    this->chunk_number_per_page = page_size / chunk_size;
    this->page_total = 0;
    this->magic_number = SLAB_MAGIC_NUMBER;
    /* Preallocate as many slab pages as possible (called from mem_cache_init)
       on start-up, so users don't get confused out-of-memory errors when
       they do have free (in-slab) space, but no space to make new mem_cache.
       if maxmem_cache is 18 (POWER_LARGEST - POWER_SMALLEST + 1), then all
       slab types can be made.  if max memory is less than 18 MB, only the
       smaller ones will be made.  */
    if (pre_alloc) {
        /* pre-allocate a 1MB slab in every size class so people don't get
           confused by non-intuitive "SERVER_ERROR out of memory"
           messages.  this is the most common question on the mailing
           list.  if you really don't want this, you can rebuild without
           these three lines.  */
       slab_new_page();
    }
}

bool moxie::Slab::slab_new_page() {
    char *ptr;
    if (this->end_page_ptr != NULL || this->end_page_free > 0) {
        return false;
    }

    ptr = (char *)calloc(1, this->page_size);
    if (NULL == ptr) {
        return false;
    }
    this->end_page_ptr = ptr;
    this->end_page_free = this->chunk_number_per_page;

    ++this->page_total;
    return true;
}

void *moxie::Slab::slab_alloc_chunk() {
    /* fail unless we have space at the end of a recently allocated page,
       we have something on our freelist, or we could allocate a new page */
    if (! (this->end_page_ptr || this->free_chunk_end || slab_new_page())) {
        return nullptr;
    }

    /* return off our freelist, if we have one */
    if (0 != this->free_chunk_end) {
        return this->free_chunk_list[--this->free_chunk_end];
    }
    /* if we recently allocated a whole page, return from that */
    if (this->end_page_ptr) {
        void *ptr = this->end_page_ptr;
        if (--this->end_page_free) {
            this->end_page_ptr = (void *)((unsigned long)this->end_page_ptr + this->chunk_size);
        } else {
            this->end_page_ptr = 0;
        }
        return ptr;
    }
    
    return 0;  /* shouldn't ever get here */
}

bool moxie::Slab::slab_free_chunk(void *ptr) {
    if (NULL == ptr) {
        return false;
    }
    if (this->free_chunk_end == this->free_chunk_list_length) { /* need more space on the free list */
        size_t new_size = this->free_chunk_list_length ? this->free_chunk_list_length * 2 : 16;  /* 16 is arbitrary */
        void **new_list = (void **)realloc(this->free_chunk_list, new_size * sizeof(void *));
        if (NULL == new_list) {
            return false;
        }
        this->free_chunk_list = new_list;
        this->free_chunk_list_length = new_size;
    }
    this->free_chunk_list[this->free_chunk_end ++] = ptr;
    return true;
}
