#ifndef MM_TYPES_H
#define MM_TYPES_H

#include "list.h"

#define MEM_TOTAL 0x40000000
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

struct mm_struct {
    unsigned long *pgd;
};


#include "kern/page.h"

#define MAX_ORDER 10

#define PHY_FRAMES_NUM (MEM_TOTAL / PAGE_SIZE)

#define PFN_2_PHY(pfn) (long)( pfn << PAGE_SHIFT )
#define PHY_2_PFN(adr) ((long)adr >> PAGE_SHIFT )

// kernel space address translation
#define VIRT_2_PHY(vaddr) ((long)vaddr & 0x0000ffffffffffff)
#define PHY_2_VIRT(vaddr) ((long)vaddr | 0xffff000000000000)

#endif