#ifndef VFS_H
#define VFS_H

#include "list.h"

#define O_CREAT 00000100

#define I_FILE      1
#define I_DIRECTORY 2

struct inode {
    struct mount            *i_mount;
    struct inode_operations *i_op;
    struct file_operations  *i_fop;
    struct dentry           *i_dentry;
    unsigned int            i_flags;
    unsigned int            i_type;
    unsigned long           i_size;
    void* internal;
};

struct dentry {         
    char                     d_name[32];
    struct dentry            *d_parent;
    struct inode             *d_inode;
    struct dentry_operations *d_op;
    struct list_head         d_child;
    struct list_head         d_subdirs;
};

struct file {
    struct inode* inode;
    long f_pos;  // RW position of this file handle
    struct file_operations* fop;
    int flags;
};


struct mount {
    struct dentry* root;
    struct filesystem* fs;
};

struct filesystem {
    char name[32];
    int (*setup_mount)(struct filesystem* fs, struct mount* mount);
};

struct file_operations {
    int (*write)(struct file* file, const void* buf, long len);
    int (*read)(struct file* file, void* buf, long len);
    int (*open)(struct inode* file_node, struct file** target);
    int (*close)(struct file* file);
    long (*lseek64)(struct file* file, long offset, int whence);
};

struct inode_operations {
    int (*lookup)(struct inode* dir_node, struct inode** target,
                const char* component_name);
    int (*create)(struct inode* dir_node, struct inode** target,
                const char* component_name);
    int (*mkdir)(struct inode* dir_node, struct inode** target,
                const char* component_name);
};

struct dentry_operations {
};

void rootfs_init();
int vfs_open(const char* pathname, int flags, struct file** target);
int vfs_close(struct file *file);
int vfs_write(struct file* file, const void* buf, long len);
int vfs_read(struct file* file, void* buf, long len);
int vfs_mkdir(const char* pathname);

#endif