#include "fs/vfs.h"
#include "fs/tmpfs.h"
#include "string.h"
#include "kern/kio.h"

struct mount* rootfs;

int register_filesystem(struct filesystem* fs) {
    if (!strcmp(fs->name, "tmpfs")) {
        return tmpfs_register();
    }
    return -1;
}

void rootfs_init() {
    struct filesystem *tmpfs = (struct filesystem *)kmalloc(sizeof(struct filesystem));
    strcpy(tmpfs->name, "tmpfs");
    tmpfs->setup_mount = tmpfs_setup_mount;
    register_filesystem(tmpfs);

    rootfs = (struct mount *)kmalloc(sizeof(struct mount));
    tmpfs->setup_mount(tmpfs, rootfs);
}

void vfs_walk_recursive(struct inode *dir_node, const char *pathname, struct inode **target, char *target_name) {
    struct list_head *ptr;
    struct dentry    *dentry;
    int i = 0;
    while(pathname[i]) {
        if (pathname[i] == '/')
            break;
        target_name[i] = pathname[i];
        i++;
    }
    target_name[i] = '\0';

    *target = dir_node;
    if (i == 0) 
        return;

    if (list_empty(&dir_node->i_dentry->d_subdirs)) 
        return;
    list_for_each(ptr, &dir_node->i_dentry->d_subdirs) {
        dentry = list_entry(ptr, struct dentry, d_child);
        if (!strcmp(dentry->d_name, target_name)) {
            if (dentry->d_inode->i_type == I_DIRECTORY)
                vfs_walk_recursive(dentry->d_inode, pathname+i+1, target, target_name);
            return;
        }
    }
}

void vfs_walk(const char *pathname, struct inode **target, char *target_name) {
    struct inode *root;
    if (pathname[0] == '/') {
        root = rootfs->root->d_inode;
        vfs_walk_recursive(root, pathname+1, target, target_name);
    }
}

struct file* create_fd(struct inode *file_node) {
    struct file *fd = (struct file*)kmalloc(sizeof(struct file));
    fd->inode = file_node;
    fd->f_pos = 0;
    fd->fop   = file_node->i_fop;
    return fd;
}


int vfs_open(const char *pathname, int flags, struct file **target) {
    // 1. Lookup pathname
    // 2. Create a new file handle for this vnode if found.
    // 3. Create a new file if O_CREAT is specified in flags and vnode not found
    // lookup error code shows if file exist or not or other error occurs
    // 4. Return error code if fails
    struct inode *dir_node;
    struct inode *file_node;
    char filename[32];
    vfs_walk(pathname, &dir_node, filename);

    if (dir_node->i_op->lookup(dir_node, &file_node, filename) >= 0) {
        *target = create_fd(file_node);
        return 0;
    } 
    
    if (flags & O_CREAT) {
        if (dir_node->i_op->create(dir_node, &file_node, filename) < 0)
            return -1;
        *target = create_fd(file_node);
        return 0;
    }
    return -1;
}

int vfs_close(struct file *file) {
    // 1. release the file handle
    // 2. Return error code if fails
    kfree(file);
    return 0;
}

int vfs_write(struct file* file, const void* buf, long len) {
    // 1. write len byte from buf to the opened file.
    // 2. return written size or error code if an error occurs.
    if (file->inode->i_type != I_FILE) {
        kprintf("vfs_write: not a regular file %d\n", file->inode->i_type);
        return -1;
    }
    return file->fop->write(file, buf, len);
}

int vfs_read(struct file* file, void* buf, long len) {
    // 1. read min(len, readable size) byte to buf from the opened file.
    // 2. block if nothing to read for FIFO type
    // 2. return read size or error code if an error occurs.
    if (file->inode->i_type != I_FILE) {
        kprintf("vfs_read: not a regular file %d\n", file->inode->i_type);
        return -1;
    }
    return file->fop->read(file, buf, len);
}

int vfs_mkdir(const char* pathname) {
    struct inode *dir_node;
    struct inode *new_dir_node;
    char dirname[32];
    vfs_walk(pathname, &dir_node, dirname);
    return dir_node->i_op->mkdir(dir_node, &new_dir_node, dirname);
}