#ifndef TMPFS_H
#define TMPFS_H

struct tmpfs_inode {
    char data[4096];
};

struct initramfs_inode {
    char *data;
};

int tmpfs_register();
struct filesystem* tmpfs_get_filesystem();
struct filesystem* initramfs_get_filesystem();
struct dentry* tmpfs_create_dentry(struct dentry *parent, const char *name, unsigned int type, unsigned int flags);

#endif