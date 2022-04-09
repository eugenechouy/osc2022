
extern unsigned int __heap_start;
extern unsigned int __heap_end;

void *heap_cur   = (void *)&__heap_start;
void *head_limit = (void *)&__heap_end;

void* simple_malloc(unsigned int size) {
    void *ret = (void*)0;
    
    size = (size + 3) & (-4);
    if (heap_cur + size >= head_limit) 
        goto out;

    ret       = heap_cur;
    heap_cur += size;
out:
    return ret;
}