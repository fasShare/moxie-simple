#include <iostream>
#include <string>
#include <assert.h>

#include <Slab.h>
#include <Items.h>
#include <MemCache.h>
#include <HashTable.h>

using namespace std;
using namespace moxie;

#define M (1014 * 1024)

int main() {
    MemCache *cache = new (std::nothrow) MemCache(48, 1.25, 40, DEFAULT_POWER_BLOCK, 512 * M * 4, 1);
    if (!cache) {
        std::cout << "New Memcache failed!" << std::endl;
        return -1;
    }

    std::string key("key_d\0emo", 9);
    std::string value("value\0_demo", 11);

    Item *it = cache->create_item(key.c_str(), key.size(), 0, value.c_str(), value.size());
    if (!it) {
        std::cout << "create_item failed!" << std::endl;
        return -1;
    }

    std::cout << "key:" << std::string(it->ITEM_key(), it->keylen()) << " key_real_len:" << std::string(it->ITEM_key(), it->keylen()).size() << std::endl;
    std::cout << "value:" << std::string(it->ITEM_data(), it->datalen()) << " value_real_len:" << std::string(it->ITEM_data(), it->datalen()).size() << std::endl;
    
    HashTable *hashtable = new HashTable(2, 1);

    assert(!hashtable->item_find(key.c_str(), 9));
    hashtable->item_insert(it);
    assert(it == hashtable->item_find(key.c_str(), 9));
    hashtable->item_delete(it);
    assert(!hashtable->item_find(key.c_str(), 9));
    assert(hashtable->item_insert(it));
    delete hashtable;

    std::cout << "--------------------------------------------------------------" << std::endl;

    HashTable *hashtable_two = new HashTable(1, 1);
    for (int i = 0; i < 10000000; ++i) {
        std::string key = std::string("key") + std::to_string(i);
        std::string value = key + key;
        Item *it = cache->create_item(key.c_str(), key.size(), 0, value.c_str(), value.size());
        if (!it) {
            std::cout << "create item failed!" << std::endl;
            return -1;
        }
        hashtable_two->item_insert(it);
        //std::cout << "hash_table_item_count = " << hashtable_two->get_item_count() << std::endl;
        assert(hashtable_two->item_find(key.c_str(), key.size()));
        if (i % 2 == 0) {
            assert(hashtable_two->item_delete(it));
            assert(!hashtable_two->item_find(key.c_str(), key.size()));
        }
    }

    std::cout << "begin find" << std::endl;
    for (int i = 0; i < 10000000; ++i) {
        std::string key = std::string("key") + std::to_string(i);
        std::string value = key + key;
        if (i % 2 == 0) {
            assert(!hashtable_two->item_find(key.c_str(), key.size()));
        } else {
            assert(hashtable_two->item_find(key.c_str(), key.size()));
        }

    }

    return 0;
}
