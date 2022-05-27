#ifndef FDTABLE_H
#define FDTABLE_H

#include "bitmap.h"
#include "fs/vfs.h"

#define MAX_OPEN_FD 16

struct files_struct {
    DECLARE_BITMAP(fd_bitmap, MAX_OPEN_FD);
    struct file *fd_array[MAX_OPEN_FD];
};

void fd_init(struct files_struct *files_struct);
int fd_open(struct files_struct *files_struct, struct file *fh);
struct file* fd_close(struct files_struct *files_struct, int fd);
struct file *fd_get(struct files_struct *files_struct, int fd);

#endif