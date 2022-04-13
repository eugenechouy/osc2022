#ifndef SLAB_H
#define SLAB_H

#include "list.h"

/*
    Fixed object size to multiple of 8 if object size less than 128. (16 pools)
    Fixed object size to multiple of 12 for next 8 pools.
    The remaining pool contain arbitrary size object. (224 bytes ~ 4096 bytes)
*/
#define MAX_OBJ_CACHE_NUM 40
#define SMALL_OBJ_SIZE 128
#define MEDIUM_OBJ_SIZE 224

struct kmem_pool {
    int object_size;  // -1 indicate free
    unsigned int gfporder;
    unsigned int num; // object per slab
    
    struct list_head slab_list;
};

struct slab_t {
    unsigned int inuse;
    unsigned int nr_free;
    void *head_addr;
    struct list_head free_list; 

    struct list_head list;
};

void slab_init();

void* kmalloc(unsigned int size);
void  kfree(void *addr);

#endif