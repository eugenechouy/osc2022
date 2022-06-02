#include "kern/fdtable.h"
#include "fs/vfs.h"

void fd_init(struct files_struct *files_struct) {
    struct file *stdio_fh;
    bitmap_zero(files_struct->fd_bitmap, MAX_OPEN_FD);

    // open /dev/uart as stdin (fd 0), stdout (fd 1), and stderr (fd 2)
    vfs_open("/dev/uart", 0, &stdio_fh);
    files_struct->fd_array[0] = stdio_fh;
    files_struct->fd_array[1] = stdio_fh;
    files_struct->fd_array[2] = stdio_fh;

    for (int i=3 ; i<MAX_OPEN_FD ; i++) 
        __set_bit(i, files_struct->fd_bitmap);
}

int fd_open(struct files_struct *files_struct, struct file *fh) {
    int fd = __ffs(files_struct->fd_bitmap[0]);
    __clear_bit(fd, files_struct->fd_bitmap);
    files_struct->fd_array[fd] = fh;
    return fd;
}

struct file* fd_close(struct files_struct *files_struct, int fd) {
    struct file *fh;
    __set_bit(fd, files_struct->fd_bitmap);
    fh = files_struct->fd_array[fd];
    files_struct->fd_array[fd] = 0;
    return fh;
}

struct file *fd_get(struct files_struct *files_struct, int fd) {
    return files_struct->fd_array[fd];
}