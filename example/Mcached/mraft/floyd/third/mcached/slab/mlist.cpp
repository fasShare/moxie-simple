#include "mlist.h"

bool moxie::moxie_list::out_of_list(struct mlist* node) {
    return (!node) || ((node->next == nullptr) && (node->prev == nullptr));
}

bool moxie::moxie_list::insert_to_list_after_pnode(struct mlist* pnode, struct mlist* node) {
    if (!node || !pnode || !out_of_list(node)) {
        return false;
    }
    node->next = pnode->next;
    if (pnode->next) {
        pnode->next->prev = node;
    }
    pnode->next = node;
    node->prev = pnode;
    return true;
}

bool moxie::moxie_list::remove_from_list(struct mlist* node) {
    if (!node || out_of_list(node)) {
        return false;
    }
    mlist *prev = node->prev;
    prev->next = node->next;
    if (prev->next) {
        prev->next->prev = prev;
    }
    node->next = nullptr;
    node->prev = nullptr;
    return true;
}

 struct moxie::moxie_list::mlist* moxie::moxie_list::remove_from_list_ret_next(struct mlist* node) {
    if (!node || out_of_list(node)) {
        return nullptr;
    }
    mlist *prev = node->prev;
    prev->next = node->next;
    if (prev->next) {
        prev->next->prev = prev;
    }
    node->next = nullptr;
    node->prev = nullptr;
    return prev->next;
}

struct moxie::moxie_list::mlist* moxie::moxie_list::remove_from_list_ret_prev(struct mlist* node) {
    if (!node || out_of_list(node)) {
        return nullptr;
    }
    mlist *prev = node->prev;
    prev->next = node->next;
    if (prev->next) {
        prev->next->prev = prev;
    }
    node->next = nullptr;
    node->prev = nullptr;
    return prev;
}