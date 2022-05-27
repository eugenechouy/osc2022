#ifndef TMPFS_H
#define TMPFS_H

struct tmpfs_inode {
    char data[4096];
};

int tmpfs_register();
struct filesystem* tmpfs_get_filesystem();

#endif