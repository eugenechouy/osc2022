#ifndef PAGECACHE_H
#define PAGECACHE_H

#include "list.h"

void  pagecache_init();

void* pagecache_read(int lba);
void  pagecache_dirty(int lba);
void  pagecache_write_back();

#endif