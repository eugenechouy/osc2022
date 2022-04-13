#ifndef MM_H
#define MM_H

#include "kern/mm_types.h"
#include "kern/slab.h"

void mm_init();

struct page* alloc_pages(unsigned int order);
void free_pages(struct page *page);

struct page frames[PHY_FRAMES_NUM];

#endif