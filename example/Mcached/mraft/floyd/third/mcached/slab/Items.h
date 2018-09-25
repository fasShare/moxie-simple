#ifndef MOXIE_ITEMS_H
#define MOXIE_ITEMS_H

typedef unsigned long long rel_time_t;
typedef unsigned char uint8_t;
const uint8_t ITEM_ALLOC = (1 << 1);
const uint8_t ITEM_HASHED = (1 << 3);

namespace moxie {

class MemCache;

class Item {
    friend class MemCache;
    friend class HashTable;
public:
    Item(MemCache *mc) {
        mc = mc;
    }

    char *ITEM_key() {
        return ((char*)&(end[0]));
    }
 
    char *ITEM_data() {
        return ((char*) &(end[0]) + nkey);
    }
    size_t ITEM_ntotal() {
        return (sizeof(Item) + nkey + nbytes);
    }

    size_t keylen() const {
        return nkey;
    }
    size_t datalen() const {
        return nbytes;
    }

    uint8_t itemflags() const {
        return it_flags;
    }

    uint8_t clearflags(uint8_t bits) {
        return it_flags &= ~bits;
    }

    uint8_t setflags(uint8_t bits) {
        return it_flags |= bits;
    }

    bool hasflags(uint8_t bits) {
        return (it_flags & bits) != 0;
    }
private:
    MemCache *mc;
    Item *h_prev;
    Item *h_next;    /* hash chain next */
    rel_time_t      time;       /* least recent access */
    rel_time_t      exptime;    /* expire time */
    size_t          nbytes;     /* size of data */
    uint8_t         refcount;
    uint8_t         it_flags;   /* ITEM_* above */
    uint8_t         slabs_clsid;/* which slab class we're in */
    size_t         nkey;       /* key length, w/terminating null and padding */
    void * end[0];
    /* then null-terminated key */
    /* then " flags length\r\n" (no terminating null) */
    /* then data with terminating \r\n (no terminating null; it's binary!) */
};

int item_make_header(size_t keylen, size_t nbytes);

}

#endif /* End definition of ITEMS_H */
