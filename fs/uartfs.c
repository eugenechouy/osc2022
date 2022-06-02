/*
    Used to register uart as device file, not a filesystem.
*/
#include "fs/vfs.h"
#include "kern/slab.h"
#include "peripheral/uart.h"

int uartfs_read(struct file *file, void *buffer, long len) {
    int i;
    char *buf = (char *)buffer;
    for(i=0 ; i<len ; i++)
        buf[i] = uart_async_read();
    return i;
}

int uartfs_write(struct file *file, const void *buffer, long len) {
    int i;
    char *buf = (char *)buffer;
    for(i=0 ; i<len ; i++) {
        if (buf[i] == '\n')
            uart_async_write('\r');
        uart_async_write(buf[i]);
    }
    return i;
}

int uartfs_open(struct inode* file_node, struct file** target) {
    return 0;
}

int uartfs_close(struct file *file) {
    return 0;
}

long uartfs_lseek64(struct file* file, long offset, int whence) {
    return 0;
}

void uartfs_register() {
    int dev;
    struct file_operations *fop = (struct file_operations*)kmalloc(sizeof(struct file_operations));
    fop->read    = uartfs_read;
    fop->write   = uartfs_write;
    fop->open    = uartfs_open;
    fop->close   = uartfs_close;
    fop->lseek64 = uartfs_lseek64;
    dev = vfs_register_device(fop);
    vfs_mknod("/dev/uart", 0, dev);
}