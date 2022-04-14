#include "kern/mm.h"

extern unsigned int __heap_start;

void *heap_start   = (void *)&__heap_start;
void *heap_cur   = (void *)&__heap_start;

int startup = 1;

void* sumalloc(unsigned int size) {
    void *ret = (void*)0;
    if (!startup)
        goto out;
    ret       = heap_cur;
    heap_cur += size;
out:
    return ret;
}

void reserved_kern_startup() {
    mm_reserve(heap_start, heap_cur);
    startup = 0;
}