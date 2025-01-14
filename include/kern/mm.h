#ifndef MM_H
#define MM_H

#include "kern/mm_types.h"
#include "kern/slab.h"

void mm_reserve(void *start, void *end);

void mm_callback(char *node_name, char *prop_name, void *prop_value);

void mm_init();

struct page* get_page_from_addr(void *addr);
struct page* alloc_pages(unsigned int order);
void free_pages(void *addr);

#endif