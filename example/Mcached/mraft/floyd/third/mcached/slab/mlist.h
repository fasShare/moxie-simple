#ifndef MOXIE_LIST_H
#define MOXIE_LIST_H

namespace moxie {

namespace moxie_list {

struct mlist {
    struct mlist *next;
    struct mlist *prev;
    mlist() :
        next(nullptr),
        prev(nullptr) {}
};

bool out_of_list(struct mlist* node);
bool insert_to_list_after_pnode(struct mlist* pnode, struct mlist* node);
bool remove_from_list(struct mlist* node);
struct mlist* remove_from_list_ret_next(struct mlist* node);
struct mlist* remove_from_list_ret_prev(struct mlist* node);
}

}

#endif 