#include "kern/mm.h"
#include "kern/kio.h"
#include "simple_alloc.h"

#define PREALLOC_SIZE 100

/*
    slab_t
*/
struct slab_t* pmalloc_slab() {
    struct slab_t *ret;
    ret = (struct slab_t *)simple_malloc(sizeof(struct slab_t));
    return ret;
}

void printslab(struct slab_t* slab) {
    kputs("New slab: ");
    kputn((long)slab->head_addr, 16);
    kputs(", ");
    kputn(slab->nr_free, 10);
    kputs("\n");
}


struct kmem_pool *kmalloc_pools;

void slab_alloc_ds() {
    kmalloc_pools = (struct kmem_pool *)simple_malloc(sizeof(struct kmem_pool) * MAX_OBJ_CACHE_NUM);
}

struct kmem_pool* kmalloc_slab(unsigned int size) {
    unsigned int rounded_size;
    int pool_id;
    
    if (size <= SMALL_OBJ_SIZE) {
        rounded_size = (size + 7) & -8;
        pool_id = rounded_size / 8 - 1;
    } else if (size <= MEDIUM_OBJ_SIZE) {
        rounded_size = (size + 11) & -12;
        pool_id = rounded_size / 12 - 1;
    } else {
        for(pool_id=24 ; pool_id<MAX_OBJ_CACHE_NUM ; pool_id++) {
            if (kmalloc_pools[pool_id].object_size == -1)
                break;
        }
        if (pool_id >= MAX_OBJ_CACHE_NUM)
            return 0;
        kmalloc_pools[pool_id].object_size = size;
        kmalloc_pools[pool_id].num         = PAGE_SIZE*4 / size;
    }

    return &kmalloc_pools[pool_id];
}

struct slab_t* slab_create(struct kmem_pool *pool) {
    struct slab_t *slab;
    struct page *page;

    page = alloc_pages(pool->gfporder);
    page->flags = PG_SLAB;

    slab = pmalloc_slab();
    slab->inuse     = 0;
    slab->nr_free   = pool->num;
    slab->head_addr = (void*)PFN_2_PHY(page->pg_index);
    INIT_LIST_HEAD(&slab->free_list);
    INIT_LIST_HEAD(&slab->list);

    page->slab = slab;

    printslab(slab);

    return slab;
}

void* slab_alloc(struct kmem_pool *pool) {
    struct slab_t *slab;
    struct list_head *ptr;
    void *ret;

    if (list_empty(&pool->slab_list)) 
        goto new_slab;
    
    list_for_each(ptr, &pool->slab_list) {
        slab = list_entry(ptr, struct slab_t, list);
        if (slab->nr_free > 0)
            break;
    }

    if (slab->nr_free == 0) {
new_slab:
        slab = slab_create(pool);
        list_add_tail(&slab->list, &pool->slab_list);
    }

    slab->inuse++;
    slab->nr_free--;
    if (!list_empty(&slab->free_list)) {
        ptr = slab->free_list.next;
        list_del(ptr);
        return (void*)ptr;
    }
    ret = slab->head_addr + (slab->inuse-1) * pool->object_size;
    return ret;
}

void* __do_kmalloc(unsigned int size) {
    struct kmem_pool *cachep;
    void *ret;

    cachep = kmalloc_slab(size);
    if (!cachep) {
        kputs("Unable to find kmem_pool...\n");
        return 0;
    }
    ret = slab_alloc(cachep);
    return ret;
}

void* kmalloc(unsigned int size) {
    int i;
    void *ret;
    struct page *page;

    if (size >= PAGE_SIZE) {
        kputs("kmalloc: boddy\n");
        for (i=0 ; i<MAX_ORDER ; i++) {
            if (size <= PAGE_SIZE * (1 << i))
                break;
        }
        page = alloc_pages(i);
        if (!page)
            return 0;
        ret = (void*)PFN_2_PHY(page->pg_index);
    } else {
        kputs("kmalloc: slab\n");
        ret = __do_kmalloc(size);
    }
    return ret;
}

void kfree(void *addr) {
    struct slab_t *slab;
    int pfn = PHY_2_PFN(addr);
    struct page page = frames[pfn];

    if (page.flags == PG_SLAB) {
        kputs("kfree: slab\n");
        slab = page.slab;
        list_add_tail((struct list_head*)addr, &slab->free_list);
        slab->inuse--;
        slab->nr_free++;
    } else {
        kputs("kfree: buddy\n");
        free_pages(&page);
    }
}


void slab_init() {
    int i;

    slab_alloc_ds();

    for (i=0 ; i<MAX_OBJ_CACHE_NUM ; i++) {
        INIT_LIST_HEAD(&kmalloc_pools[i].slab_list);
        if (i < 16) {
            kmalloc_pools[i].object_size = (i+1) * 8;
            kmalloc_pools[i].gfporder    = 0;
            kmalloc_pools[i].num         = PAGE_SIZE / kmalloc_pools[i].object_size;
        }
        else if (i < 24) {
            kmalloc_pools[i].object_size = SMALL_OBJ_SIZE + (i-16+1) * 12;
            kmalloc_pools[i].gfporder    = 1;
            kmalloc_pools[i].num         = PAGE_SIZE*2 / kmalloc_pools[i].object_size;
        }   
        else {
            kmalloc_pools[i].object_size = -1;
            kmalloc_pools[i].gfporder    = 2;
            kmalloc_pools[i].num         = 0;
        }
    }
}