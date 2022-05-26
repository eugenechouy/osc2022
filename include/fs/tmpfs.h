#ifndef TMPFS_H
#define TMPFS_H

struct tmpfs_inode {
    char data[4096];
};

int tmpfs_register();
int tmpfs_setup_mount(struct filesystem *fs, struct mount *mount);

#endif