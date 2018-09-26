#ifndef ASSOC_H
#define ASSOC_H
#include "Items.h"

namespace moxie {

class HashTable {
public:
    HashTable(size_t power, double factor);
    ~HashTable();
    moxie::Item *item_find(const char *key, uint32_t keylen);
    bool item_insert(Item *item);
    bool item_delete(Item *item);
    moxie::Item* item_delete(const char *key, uint32_t keylen);
    Item *get_bucket_from_key(const char *key, uint32_t keylen);
    uint64_t get_item_count() const { return item_count; }
    Item* insert_to_bucket(Item* bucket, Item *item);
    Item* remove_from_bucket(Item* bucket, Item *item);
    bool need_expand() const;
    bool expand_table();
    unsigned long hash_index(const char* key, size_t keylen, size_t power) const;
private:
    Item** hashtable;
    uint64_t hash_size;
    uint64_t item_count;
    size_t power;
    double factor;

    uint64_t expand_index;
    size_t new_power;
    bool is_expand;
    size_t new_hashsize;
    Item** new_hashtable;
};

}
#endif
