#ifndef TMPFS_H
#define TMPFS_H

struct tmpfs_inode {
    char data[4096];
};

struct initramfs_inode {
    char *data;
};

struct filesystem* tmpfs_get_filesystem();
struct filesystem* initramfs_get_filesystem();

#endif