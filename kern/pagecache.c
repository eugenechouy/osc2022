/*
    pagecache used for FAT32 partition only
*/
#include "kern/kio.h"
#include "kern/slab.h"
#include "kern/pagecache.h"
#include "drivers/sdhost.h"
#include "string.h"

struct filepage {
    unsigned int lba;
    void *data;
    struct list_head list;
};

struct list_head clean_list;
struct list_head dirty_list;

struct filepage *lba_map;
unsigned int offset;

#define LBA_MAP_MAX 50000
#define LBA_OFFSET  2048

void pagecache_init() {
    INIT_LIST_HEAD(&clean_list);
    INIT_LIST_HEAD(&dirty_list);
    lba_map = (struct filepage *)kmalloc(sizeof(struct filepage) * LBA_MAP_MAX);
    memset(lba_map, 0, sizeof(struct filepage) * LBA_MAP_MAX);
}

void* pagecache_read(int lba) {
    if (lba_map[lba - LBA_OFFSET].data == 0) {
        lba_map[lba - LBA_OFFSET].data = kmalloc(BLOCK_SIZE);
        lba_map[lba - LBA_OFFSET].lba  = lba;
        readblock(lba, lba_map[lba - LBA_OFFSET].data);
        // add to clean page list
        INIT_LIST_HEAD(&lba_map[lba - LBA_OFFSET].list);
        list_add_tail(&lba_map[lba - LBA_OFFSET].list, &clean_list);
    }
#ifdef DEBUG_PAGECACHE
    kprintf("read pagecache %d\n", lba);
#endif
    return lba_map[lba - LBA_OFFSET].data;
}

void pagecache_dirty(int lba) {
    if (lba_map[lba - LBA_OFFSET].data == 0) {
        kprintf("Warning: try to mark uncached data dirty\n");
        return;
    }   
#ifdef DEBUG_PAGECACHE
    kprintf("write pagecache %d\n", lba);
#endif
    list_del(&lba_map[lba - LBA_OFFSET].list);
    list_add_tail(&lba_map[lba - LBA_OFFSET].list, &dirty_list);
}

void pagecache_write_back() {
    struct list_head *ptr;
    struct list_head *tmp;
    struct filepage *dpage;
    list_for_each_safe(ptr, tmp, &dirty_list) {
        dpage = list_entry(ptr, struct filepage, list);
        writeblock(dpage->lba, dpage->data);
        list_del(&dpage->list);
        list_add_tail(&dpage->list, &clean_list);
    }
}
