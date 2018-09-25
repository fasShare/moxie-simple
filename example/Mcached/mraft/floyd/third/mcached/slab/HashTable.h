#ifndef ASSOC_H
#define ASSOC_H
#include "Items.h"

namespace moxie {

class HashTable {
public:
    HashTable(size_t power, double factor);
    moxie::Item *item_find(const char *key, uint32_t keylen);
    bool item_insert(Item *item);
    bool item_delete(Item *item);
private:
    Item** hashtable;
    uint64_t hash_size;
    uint64_t item_count;
    size_t power;
    double factor;
};

}
#endif
