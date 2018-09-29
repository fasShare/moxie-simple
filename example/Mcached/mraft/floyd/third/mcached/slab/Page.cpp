#include <assert.h>
#include <memory>
#include <iostream>

#include "Page.h"

moxie::Page::Page(size_t page_size, size_t chunk_size) {
    this->chunk_size = chunk_size;
    this->page_size = page_size;

    this->chunk_num = this->page_size / (this->chunk_size + sizeof(Page *));
    this->ptr = malloc(this->page_size);
    if (!ptr) {
        this->page_size = 0;
        return;
    }

    this->free_chunk_list = (void **)malloc(chunk_num * sizeof(Page *));
    if (this->free_chunk_list == nullptr) {
        this->page_size = 0;
        free(this->ptr);
        return;
    }

    for (size_t i = 0; i < this->chunk_num; ++i) {
        this->free_chunk_list[i] = (void *)((const char *)this->ptr + i * (this->chunk_size + sizeof(Page *)));
    }
    this->free_chunk_end = this->chunk_num;
}

moxie::Page::~Page() {
    if (this->free_chunk_list) {
        free(this->free_chunk_list);
    }
    if (this->ptr) {
        free(this->ptr);
    }
}

bool moxie::Page::reset(size_t chunk_size_new) {
    if (!this->ptr || !this->is_empty() || !moxie_list::out_of_list(&this->plist)) {
        return false;
    }

    if (this->page_size < (chunk_size_new + sizeof(Page *))) {
        return false;
    }

    this->chunk_size = chunk_size_new;
    this->chunk_num = this->page_size / (this->chunk_size + sizeof(Page *));
    this->free_chunk_list = (void **)realloc(this->free_chunk_list, this->chunk_num);
    if (!this->free_chunk_list) {
        return false;
    }

    for (size_t i = 0; i < this->chunk_num; ++i) {
        this->free_chunk_list[i] = (void *)((const char *)this->ptr + i * (this->chunk_size + sizeof(Page *)));
    }
    this->free_chunk_end = this->chunk_num;
    this->plist.next = this->plist.prev = nullptr;
    return true;
}

bool moxie::Page::vaild() const {
    return this->page_size != 0;
}

bool moxie::Page::is_empty() const {
    return this->free_chunk_end == this->chunk_num;
}

bool moxie::Page::is_full() const {
    return this->free_chunk_end == 0;
}

void *moxie::Page::alloc_chunk() {
    if (this->free_chunk_end--) {
        Page **ptr = (Page **)(this->free_chunk_list[this->free_chunk_end]);
        ptr[0] = this;
        return ptr + 1;
    } else {
        return nullptr;
    }
    return nullptr;
}

bool moxie::Page::free_chunk(void *chunk) {
    assert(PAGE_FROM_CHUNK(chunk) == this);
    this->free_chunk_list[this->free_chunk_end++] = chunk;
    return true;
}