#include "fs/vfs.h"
#include "fs/tmpfs.h"
#include "fs/uartfs.h"
#include "fs/fat32.h"
#include "string.h"
#include "kern/kio.h"
#include "kern/sched.h"
#include "kern/slab.h"
#include "kern/pagecache.h"

struct mount* rootfs;

/*
    device file
*/
#define MAX_DEVICE_FILE 10

int    device_id;
struct file_operations *device_files[MAX_DEVICE_FILE];


void rootfs_init() {
    struct filesystem *tmpfs = tmpfs_get_filesystem();

    rootfs = (struct mount *)kmalloc(sizeof(struct mount));
    tmpfs->setup_mount(tmpfs, rootfs);

    // device file init
    device_id = 0;
    memset(device_files, 0, MAX_DEVICE_FILE);
    vfs_mkdir("/dev");
    uartfs_register();
}

void vfs_walk_recursive(struct inode *dir_node, const char *pathname, struct inode **target, char *target_name) {
    struct list_head *ptr;
    struct dentry    *dentry;
    int i = 0;

    if (dir_node->i_dentry->d_cached == 0) {
        dir_node->i_dentry->d_op->read(dir_node->i_dentry);
    }

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
    if (!strcmp(target_name, ".")) {
        vfs_walk_recursive(dir_node, pathname+i+1, target, target_name);
        return;
    }
    if (!strcmp(target_name, "..")) {
        if (dir_node->i_dentry->d_parent == 0)
            vfs_walk_recursive(dir_node, pathname+i+1, target, target_name);
        else
            vfs_walk_recursive(dir_node->i_dentry->d_parent->d_inode, pathname+i+1, target, target_name);
        return;
    }

    if (list_empty(&dir_node->i_dentry->d_subdirs)) 
        return;
    list_for_each(ptr, &dir_node->i_dentry->d_subdirs) {
        dentry = list_entry(ptr, struct dentry, d_child);
        if (!strcmp(dentry->d_name, target_name)) {
            if (dentry->d_mount != 0) {
                vfs_walk_recursive(dentry->d_mount->root->d_inode, pathname+i+1, target, target_name);
                return;
            }
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
    } else {
        root = get_current()->cwd->d_inode;
        vfs_walk_recursive(root, pathname, target, target_name);
    }
}

struct file* create_fh(struct inode *file_node) {
    struct file *fh = (struct file*)kmalloc(sizeof(struct file));
    fh->inode = file_node;
    fh->f_pos = 0;
    fh->fop   = file_node->i_fop;
    fh->cur_clus = 1; // cluster number start with 2
    return fh;
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
        *target = create_fh(file_node);
        return 0;
    } 
    // kprintf("%s not found under %s\n", pathname, dir_node->i_dentry->d_name);
    
    if (flags & O_CREAT) {
        if (dir_node->i_flags == I_FRO) 
            return -3;
        if (dir_node->i_op->create(dir_node, &file_node, filename) < 0)
            return -1;
        *target = create_fh(file_node);
        return 0;
    }
    return -1;
}

int vfs_close(struct file *file) {
    // 1. release the file handle
    // 2. Return error code if fails
    if (file == 0)
        return -1;
    kfree(file);
    return 0;
}

int vfs_write(struct file* file, const void* buf, long len) {
    // 1. write len byte from buf to the opened file.
    // 2. return written size or error code if an error occurs.
    if (file == 0)
        return -1;
    if (file->inode->i_type != I_FILE) {
        kprintf("vfs_write: not a regular file %d\n", file->inode->i_type);
        return -1;
    }
    if (file->inode->i_flags == I_FRO) {
        return -3;
    }
    return file->fop->write(file, buf, len);
}

int vfs_read(struct file* file, void* buf, long len) {
    // 1. read min(len, readable size) byte to buf from the opened file.
    // 2. block if nothing to read for FIFO type
    // 2. return read size or error code if an error occurs.
    if (file == 0)
        return -1;
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
    if (dir_node->i_type != I_DIRECTORY) {
        kprintf("vfs_mkdir: not a directory %d\n", dir_node->i_type);
        return -1;
    }
    if (dir_node->i_flags == I_FRO) {
        return -3;
    }
    return dir_node->i_op->mkdir(dir_node, &new_dir_node, dirname);
}

int vfs_mount(const char* target, const char* filesystem) {
    struct inode *mountpoint;
    struct mount *mt;
    struct filesystem *fs;
    char remain[32];
    vfs_walk(target, &mountpoint, remain);

    if (mountpoint->i_type != I_DIRECTORY) 
        return -1;
    if (!strcmp(mountpoint->i_dentry->d_name, "/")) {
        kprintf("%s already been mounted\n", target);
        return -2;
    }
    
    if (!strcmp(filesystem, "tmpfs")) {
        fs = tmpfs_get_filesystem();
    } else if (!strcmp(filesystem, "initramfs")) {
        fs = initramfs_get_filesystem();
    } else if (!strcmp(filesystem, "fat32")) {
        fs = fat32_get_filesystem();
    } else {
        kprintf("Unsupported filesystem: %s\n", filesystem);
        return -3;
    }
    mt = (struct mount *)kmalloc(sizeof(struct mount));
    fs->setup_mount(fs, mt);
    mountpoint->i_dentry->d_mount = mt;
    mt->root->d_parent = mountpoint->i_dentry->d_parent;
    return 0;
}

int vfs_chdir(const char *pathname) {
    struct inode *dir_node;
    char remain[32];
    vfs_walk(pathname, &dir_node, remain);
    get_current()->cwd = dir_node->i_dentry;
    return 0;
}

long vfs_lseek64(struct file *file, long offset, int whence) {
    if (file == 0) 
        return -1;
    if (file->inode->i_type != I_FILE) {
        kprintf("vfs_lseek64: not a regular file %d\n", file->inode->i_type);
        return -1;
    }
    return file->fop->lseek64(file, offset, whence);
}

int vfs_mknod(const char* pathname, int mode, int dev) {
    // 1. create special file
    // 2. change special file's operation to dev operation
    struct inode *dir_node;
    struct inode *file_node;
    char filename[32];
    vfs_walk(pathname, &dir_node, filename);
    
    if (dir_node->i_op->lookup(dir_node, &file_node, filename) >= 0) {
        kprintf("vfs_mknod: %s already exist\n", pathname);
        return -1;
    } 
    if (dir_node->i_flags == I_FRO) 
        return -3;
    if (dir_node->i_op->create(dir_node, &file_node, filename) < 0)
        return -1;
    file_node->i_fop = device_files[dev];
    return 0;
}

int vfs_register_device(struct file_operations *device_fop) {
    // register device operation and return device id
    if (device_id == MAX_DEVICE_FILE)
        return -1;
    device_files[device_id] = device_fop;
    return device_id++;
}

void vfs_sync() {
    pagecache_write_back();
}