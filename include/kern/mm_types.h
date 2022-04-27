#ifndef MM_TYPES_H
#define MM_TYPES_H

#include "list.h"

#define MEM_LIMIT 0x3B400000

struct free_area {
    struct list_head free_list;
    unsigned int nr_free;
    unsigned int order;
};

#define PG_USED 0
#define PG_HEAD 1
#define PG_TAIL 2
#define PG_SLAB 3

struct page {
    unsigned int flags;
    unsigned int pg_index;
    unsigned int compound_order;
    
    struct slab_t *slab; 

    struct list_head list;
};


#define MAX_ORDER 10

#define PAGE_SIZE 4096
#define PAGE_SHIFT 12
#define PHY_FRAMES_NUM 262144 // 0x40000000 / 4096

#define PFN_2_PHY(pfn) (long)( pfn << PAGE_SHIFT )
#define PHY_2_PFN(adr) ((long)adr >> PAGE_SHIFT )


#endif