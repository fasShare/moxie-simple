/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * Hash table
 *
 * The hash function used here is by Bob Jenkins, 1996:
 *    <http://burtleburtle.net/bob/hash/doobs.html>
 *       "By Bob Jenkins, 1996.  bob_jenkins@burtleburtle.net.
 *       You may use this code any way you wish, private, educational,
 *       or commercial.  It's free."
 *
 * The rest of the file is licensed under the BSD license.  See LICENSE.
 *
 * $Id$
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <memory>
#include <iostream>

#include "HashTable.h"
#include "Items.h"

typedef  unsigned long  int  ub4;   /* unsigned 4-byte quantities */
typedef  unsigned       char ub1;   /* unsigned 1-byte quantities */

/* hard-code one million buckets, for now (2**20 == 4MB hash) */
#define HASHPOWER  20
#define hashsize(n) ((ub4)1<<(n))
#define hashmask(n) (hashsize(n)-1)

#define mix(a,b,c) { \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}

/*
--------------------------------------------------------------------
hash() -- hash a variable-length key into a 32-bit value
  k       : the key (the unaligned variable-length array of bytes)
  len     : the length of the key, counting by bytes
  initval : can be any 4-byte value
Returns a 32-bit value.  Every bit of the key affects every bit of
the return value.  Every 1-bit and 2-bit delta achieves avalanche.
About 6*len+35 instructions.

The best hash table sizes are powers of 2.  There is no need to do
mod a prime (mod is sooo slow!).  If you need less than 32 bits,
use a bitmask.  For example, if you need only 10 bits, do
  h = (h & hashmask(10));
In which case, the hash table should have hashsize(10) elements.

If you are hashing n strings (ub1 **)k, do it like this:
  for (i=0, h=0; i<n; ++i) h = hash( k[i], len[i], h);

By Bob Jenkins, 1996.  bob_jenkins@burtleburtle.net.  You may use this
code any way you wish, private, educational, or commercial.  It's free.

See http://burtleburtle.net/bob/hash/evahash.html
Use for hash table lookup, or anything where one collision in 2^^32 is
acceptable.  Do NOT use for cryptographic purposes.
--------------------------------------------------------------------
*/

/* the key */
/* the length of the key */
/* the previous hash, or an arbitrary value */
ub4 hash(register ub1 *k, register ub4  length, register ub4  initval) {
    register ub4 a,b,c,len;

    /* Set up the internal state */
    len = length;
    a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
    c = initval;         /* the previous hash value */

    /*---------------------------------------- handle most of the key */
    while (len >= 12)
        {
            a += (k[0] +((ub4)k[1]<<8) +((ub4)k[2]<<16) +((ub4)k[3]<<24));
            b += (k[4] +((ub4)k[5]<<8) +((ub4)k[6]<<16) +((ub4)k[7]<<24));
            c += (k[8] +((ub4)k[9]<<8) +((ub4)k[10]<<16)+((ub4)k[11]<<24));
            mix(a,b,c);
            k += 12; len -= 12;
        }

    /*------------------------------------- handle the last 11 bytes */
    c += length;
    switch(len)              /* all the case statements fall through */
        {
        case 11: c+=((ub4)k[10]<<24);
        case 10: c+=((ub4)k[9]<<16);
        case 9 : c+=((ub4)k[8]<<8);
            /* the first byte of c is reserved for the length */
        case 8 : b+=((ub4)k[7]<<24);
        case 7 : b+=((ub4)k[6]<<16);
        case 6 : b+=((ub4)k[5]<<8);
        case 5 : b+=k[4];
        case 4 : a+=((ub4)k[3]<<24);
        case 3 : a+=((ub4)k[2]<<16);
        case 2 : a+=((ub4)k[1]<<8);
        case 1 : a+=k[0];
            /* case 0: nothing left to add */
        }
    mix(a,b,c);
    /*-------------------------------------------- report the result */
    return c;
}

moxie::HashTable::HashTable(size_t power, double factor) {
    this->hash_size = hashsize(power);
    this->hashtable = new (std::nothrow) Item*[this->hash_size];
    if (!this->hashtable) {
        fprintf(stderr, "Failed to init hashtable.\n");
        exit(1);
    }
    this->power = power;
    this->factor = factor;
    this->item_count = 0;
    for (size_t i = 0; i < this->hash_size; ++i) {
        this->hashtable[i] = nullptr;
    }
}

moxie::Item *moxie::HashTable::item_find(const char *key, uint32_t keylen) {
    ub4 hv = hash((ub1 *)key, keylen, 0) & hashmask(this->power);
    
    std::cout << hv << std::endl;
    Item *it = this->hashtable[hv];

    while (it) {
        if ((it->nkey == keylen) && (memcmp(it->ITEM_key(), key, it->nkey)) == 0)
            return it;
        it = it->h_next;
    }
    return nullptr;
}

bool moxie::HashTable::item_insert(moxie::Item *item) {
    if (!item) {
        return false;
    }
    assert((item_find(item->ITEM_key(), item->keylen()) == nullptr) 
            && !item->hasflags(ITEM_HASHED)
            && item->hasflags(ITEM_ALLOC)); 
    ub4 hv = hash((ub1 *)item->ITEM_key(), item->keylen(), 0) & hashmask(this->power);
    item->h_next = hashtable[hv];
    if (hashtable[hv]) {
        hashtable[hv]->h_prev = item;
    }
    item->h_prev = nullptr;
    hashtable[hv] = item;
    item->setflags(ITEM_HASHED);

    ++this->item_count;
    return true;
}

bool moxie::HashTable::item_delete(moxie::Item *item) {
    assert((item_find(item->ITEM_key(), item->keylen()) != nullptr) 
            && item->hasflags(ITEM_HASHED)
            && item->hasflags(ITEM_ALLOC));
    if (item->h_prev == nullptr) { // first node in double list
        ub4 hv = hash((ub1 *)item->ITEM_key(), item->keylen(), 0) & hashmask(this->power);
        hashtable[hv] = item->h_next;
        if (hashtable[hv]) {
            hashtable[hv]->h_prev = nullptr; 
        }
    } else {
        item->h_prev->h_next = item->h_next;
    }
    item->clearflags(ITEM_HASHED);

    --this->item_count;
    return true;
}

