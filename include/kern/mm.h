#ifndef MM_H
#define MM_H

#include "kern/mm_types.h"
#include "kern/slab.h"

void mm_callback(char *node_name, char *prop_name, void *prop_value);

void mm_init();

struct page* alloc_pages(unsigned int order);
void free_pages(struct page *page);

extern struct page *frames;

#endif