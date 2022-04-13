/*
memory layout

+------------------+
| Arm Peripherals  |
+------------------+ <- 0x40000000
| GPU Peripherals  |
+------------------+ <- 0x3f000000
|     reserved     |
+------------------+ <- 0x3C000000
|   User frame[n]  |
+------------------+
|        .         |
|        .         |
+------------------+
|   User frame[0]  |
+------------------+
|  Kernel frame[n] |
+------------------+
|        .         |
|        .         |
+------------------+
|  Kernel frame[0] |
+------------------+
| kernel/user data |
+------------------+
| kernel/user text |
+------------------+ <- kernel_base
*/

#include "kern/mm.h"
#include "kern/kio.h"

struct free_area free_area[MAX_ORDER];

void printpg(struct page *page) {
    kputn(page->pg_index, 10);
    kputs(", ");
    kputn(page->pg_index*PAGE_SIZE, 16);
    kputs(", ");
    kputn(page->compound_order, 10);
    kputs("\n");
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

    kputs("Release "); 
    kputn(porder, 10);
    kputs(", ask for "); 
    kputn(order, 10);
    kputs("\n");

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
            printpg(page);
            if (page)
                return page;
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

    kputs("Free: ");
    printpg(page);

    while(order < MAX_ORDER) {
        buddy = find_buddy(page);
        if (!buddy || buddy->flags != PG_HEAD || buddy->compound_order != order) {
            page->flags = PG_HEAD;
            add_to_free_area(page, &free_area[order]);
            break;
        }
        kputs("Buddy: ");
        printpg(buddy);
        kputs("Merge: ");
        kputn(order, 10);
        kputs(" to ");
        kputn(order+1, 10);
        kputs("\n");

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

    int kernel_data_end   = __heap_start / PAGE_SIZE;
    int mem_end           = MEM_LIMIT / PAGE_SIZE;

    for (i=0 ; i<MAX_ORDER ; i++) {
        INIT_LIST_HEAD(&free_area[i].free_list);
        free_area[i].nr_free = 0;
    }

    for (i=0 ; i<=kernel_data_end ; i++) {
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

void test_mm() {
    struct page *page[10];
    for(int i=0 ; i<10 ; i++) {
        page[i] = alloc_pages(0);
        printpg(page[i]);
    }

    kputs("\n\n\n");

    for(int i=0 ; i<10 ; i++) {
        free_pages(page[i]);
    }
}