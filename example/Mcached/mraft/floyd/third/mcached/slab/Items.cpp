/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* $Id$ */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <time.h>
#include <assert.h>

#include "Items.h"
#include "MemCache.h"
#include "HashTable.h"

int moxie::item_make_header(size_t keylen, size_t nbytes) {
    return sizeof(moxie::Item) + keylen + nbytes;
}