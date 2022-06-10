#ifndef SDHOST_H
#define SDHOST_H

void readblock(int block_idx, void* buf);
void writeblock(int block_idx, void* buf);
void sd_init();

#define BLOCK_SIZE 512

#endif