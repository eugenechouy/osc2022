#include "kern/mm.h"
#include "kern/kio.h"
#include "startup_alloc.h"

#include "string.h"
#include "byteswap.h"

unsigned int phy_address_start = 0;
unsigned int phy_address_limit = 0x3C000000;

void mm_callback(char *node_name, char *prop_name, void *prop_value) {
    if (!strncmp(node_name, "memory", 6) && !strncmp(prop_name, "reg", 3)) {
        kputs("mm: Find!\n");
        phy_address_start = __bswap_32(*((unsigned int *)(prop_value)));
        phy_address_limit = __bswap_32(*((unsigned int *)(prop_value+4)));
        kprintf("physical start address: 0x%x\n", phy_address_start);
        kprintf("physical address limit: 0x%x\n", phy_address_limit);
    }
}

struct free_area *free_area;
struct page *frames;

void buddy_alloc_ds() {
    frames = (struct page *)sumalloc(sizeof(struct page) * PHY_FRAMES_NUM);
    free_area = (struct free_area *)sumalloc(sizeof(struct free_area) * MAX_ORDER);
}

void add_to_free_area(struct page *page, struct free_area *free_area) {
    free_area->nr_free++;
    list_add_tail(&page->list, &free_area->free_list);
} 

void del_page_from_free_area(struct page *page, struct free_area *free_area) {
    free_area->nr_free--;
    list_del(&page->list);
}

struct page* expand(struct page *page, unsigned int order) {
    struct page *redundant;
    unsigned int porder = page->compound_order;

    // kprintf("Release %d, ask for, %d\n", porder, order);
    if (porder > order) {
        porder--;
        redundant = page + (1 << porder);
        page->compound_order = porder;
        redundant->flags          = PG_HEAD;
        redundant->compound_order = porder;
        add_to_free_area(redundant, &free_area[porder]);
        return expand(page, order);
    }
    page->flags = PG_USED;
    return page;
}

struct page* rmqueue(struct free_area *free_area, unsigned int order) {
    struct page *hpage;
    if (list_empty(&free_area->free_list))
        return 0;
    hpage = list_entry(free_area->free_list.next, struct page, list);
    del_page_from_free_area(hpage, free_area);
    return expand(hpage, order);
}

struct page* alloc_pages(unsigned int order) {
    struct page *page;
    if (order >= MAX_ORDER)
        return 0;
    for (int i=order ; i<MAX_ORDER ; i++) {
        if (free_area[i].nr_free > 0) {
            page = rmqueue(&free_area[i], order);
            if (page) {
                // kprintf("Alloc new buddy: %d, %x, %d\n", page->pg_index, page->pg_index*PAGE_SIZE, page->compound_order);
                return page;
            }
        }
    }
    return 0;
}

/*
You can use the block’s index xor with its exponent to find its buddy. 
If its buddy is in the page frame array, then you can merge them to a larger block.
*/
struct page* find_buddy(struct page *page) {
    int buddy_index = page->pg_index ^ (1 << page->compound_order);
    if (buddy_index >= PHY_FRAMES_NUM)
        return 0;
    return &frames[buddy_index];
}

void free_pages(struct page *page) {
    struct page *buddy;
    int order = page->compound_order;

    // kprintf("Free buddy: %d, %x, %d\n", page->pg_index, page->pg_index*PAGE_SIZE, page->compound_order);
    while(order < MAX_ORDER) {
        buddy = find_buddy(page);
        if (!buddy || buddy->flags != PG_HEAD || buddy->compound_order != order) {
            page->flags = PG_HEAD;
            add_to_free_area(page, &free_area[order]);
            break;
        }
        // kprintf("\tBuddy page: %d, %x, %d\n", buddy->pg_index, buddy->pg_index*PAGE_SIZE, buddy->compound_order);
        // kprintf("\tMerge %d to %d\n", order, order+1);

        // order == buddy->compound_order
        del_page_from_free_area(buddy, &free_area[order]);
        if (buddy > page) { // | page | buddy |
            page->compound_order = order+1;
            buddy->flags = PG_TAIL;
        } else { // | buddy | page |
            buddy->compound_order = order+1;
            page->flags = PG_TAIL;
            page = buddy;
        }
        order = page->compound_order;
    }
}

// start of kernel frame
extern unsigned int __heap_start; 

void mm_init() {
    int i;
    int cnt = 0;
    int order = MAX_ORDER - 1;

    int kernel_data_end   = (long)(&__heap_start) / PAGE_SIZE;
    int mem_end           = phy_address_limit / PAGE_SIZE - 10;

    buddy_alloc_ds();

    i = phy_address_start / PAGE_SIZE;
    for ( ; i<MAX_ORDER ; i++) {
        INIT_LIST_HEAD(&free_area[i].free_list);
        free_area[i].nr_free = 0;
    }

    for (i=0 ; i<kernel_data_end ; i++) {
        frames[i].flags          = PG_USED;
        frames[i].pg_index       = i;
        frames[i].compound_order = 0;
        frames[i].slab           = 0;
        INIT_LIST_HEAD(&frames[i].list);
    }

    for ( ; i<mem_end ; i++) {
        frames[i].pg_index       = i;
        frames[i].slab           = 0;
        INIT_LIST_HEAD(&frames[i].list);
        if (!cnt) {
            while (i + (1 << order) - 1 >= mem_end)
                order--;
            frames[i].flags = PG_HEAD;
            frames[i].compound_order = order;
            add_to_free_area(&frames[i], &free_area[order]);
            cnt = (1 << order) - 1;
        } else {
            frames[i].flags = PG_TAIL;     
            frames[i].compound_order = 0;    
            cnt--;
        }
    }

    for ( ; i<PHY_FRAMES_NUM ; i++) {
        frames[i].flags          = PG_USED;
        frames[i].pg_index       = i;
        frames[i].compound_order = 0;
        frames[i].slab           = 0;
        INIT_LIST_HEAD(&frames[i].list);
    }

    slab_init();
}


void expand_reserve(struct page *page, unsigned int l, unsigned int r) {
    struct page *buddy;
    unsigned int porder = page->compound_order;
    unsigned int cl     = page->pg_index;
    unsigned int cr     = cl + (1 << porder) - 1;
    unsigned int mid;

    if (cl > r || cr < l) {
        add_to_free_area(page, &free_area[porder]);
        return;
    }
    // no need to split
    if (cl >= l && cr <= r) {
        kprintf("\tReserved range: %d-%d\n", cl, cr);
        page->flags = PG_USED;
        return;
    }
    // split
    mid = (cl + cr) >> 1;
    porder--;
    buddy = &frames[mid+1];
    page->compound_order  = porder;
    buddy->flags          = PG_HEAD;
    buddy->compound_order = porder;
    expand_reserve(page, l, r);
    expand_reserve(buddy, l, r);
}

// Mark the page from start to end as used
void mm_reserve(void *start, void *end) {
    unsigned int ps = PHY_2_PFN(start);
    unsigned int pe = PHY_2_PFN(end);
    unsigned int pi = ps;
    struct page page;   
    int i;

    kprintf("Reserve %d-%d (%x-%x)\n", ps, pe, (long)start, (long)end);
    for (i=ps ; i<=pe ; i++) if (frames[i].flags == PG_USED) {
        kputs("Try to reserved page that are already been used...\n");
        return;
    }

    do {
        // find the header page of start address, linear search for now
        while(pi >= 0) {
            page = frames[pi--];
            if (page.flags == PG_HEAD)
                break;
        }
        pi = page.pg_index + (1<<page.compound_order);
        del_page_from_free_area(&page, &free_area[page.compound_order]);
        expand_reserve(&page, ps, pe);
    } while(pi <= pe);
}