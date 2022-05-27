#include "fs/vfs.h"
#include "fs/tmpfs.h"
#include "kern/slab.h"
#include "string.h"

struct inode_operations *tmpfs_iop;
struct file_operations  *tmpfs_fop;
struct dentry_opartions *tmpfs_dop;

struct inode* tmpfs_create_inode(struct dentry *dentry, unsigned int type);
struct dentry* tmpfs_create_dentry(struct dentry *parent, const char *name, unsigned int type);

/*
    inode operation
*/
int tmpfs_lookup(struct inode* dir_node, struct inode** target, const char* component_name) {
    struct list_head *ptr;
    struct dentry *dentry;
    if (list_empty(&dir_node->i_dentry->d_subdirs)) 
        return -1;
    list_for_each(ptr, &dir_node->i_dentry->d_subdirs) {
        dentry = list_entry(ptr, struct dentry, d_child);
        if (!strcmp(dentry->d_name, component_name) && dentry->d_inode->i_type == I_FILE) {
            *target = dentry->d_inode;
            return 0;
        }
    }
    return -1;
}

int tmpfs_create(struct inode* dir_node, struct inode** target, const char* component_name) {
    struct tmpfs_inode *tmpfs_internal = (struct tmpfs_inode *)kmalloc(sizeof(struct tmpfs_inode));
    struct dentry *new_dentry = tmpfs_create_dentry(dir_node->i_dentry, component_name, I_FILE);
    new_dentry->d_inode->internal = (void*)tmpfs_internal;
    *target = new_dentry->d_inode;
    return 0;
}

int tmpfs_mkdir(struct inode* dir_node, struct inode** target, const char* component_name) {
    struct dentry *new_dentry = tmpfs_create_dentry(dir_node->i_dentry, component_name, I_DIRECTORY);
    *target = new_dentry->d_inode;
    return 0;
}

/*
    file operation
*/
int tmpfs_open(struct inode* file_node, struct file** target) {
    return 0;
}

int tmpfs_close(struct file *file) {
    return 0;
}

int tmpfs_write(struct file *file, const void *buf, long len) {
    struct tmpfs_inode *tmpfs_internal = (struct tmpfs_inode *)file->inode->internal;
    unsigned long size = file->inode->i_size;
    unsigned long i    = file->f_pos;
    unsigned long bi   = 0;
    char *src = (char*)buf;
    for ( ; bi<len && src[bi] ; i++) {
        tmpfs_internal->data[i] = src[bi];
        bi++;
    }
    file->f_pos = i;
    if (i > size)
        file->inode->i_size = i;
    return bi;
}

int tmpfs_read(struct file *file, void *buf, long len) {
    struct tmpfs_inode *tmpfs_internal = (struct tmpfs_inode *)file->inode->internal;
    unsigned long size = file->inode->i_size;
    unsigned long i    = file->f_pos;
    unsigned long bi   = 0;
    char *dest = (char*)buf;
    for ( ; bi<len && i<size ; i++) {
        dest[bi] = tmpfs_internal->data[i];
        bi++;
    }
    file->f_pos = i;
    return bi;
}


int tmpfs_register() {
    tmpfs_iop = (struct inode_operations *)kmalloc(sizeof(struct inode_operations));
    tmpfs_iop->lookup = tmpfs_lookup;
    tmpfs_iop->create = tmpfs_create;
    tmpfs_iop->mkdir  = tmpfs_mkdir;
    tmpfs_fop = (struct file_operations *)kmalloc(sizeof(struct file_operations));
    tmpfs_fop->write  = tmpfs_write;
    tmpfs_fop->read   = tmpfs_read;
    tmpfs_fop->open   = tmpfs_open;
    tmpfs_fop->close  = tmpfs_close;
    tmpfs_dop = 0;
    return 0;
}

int tmpfs_setup_mount(struct filesystem *fs, struct mount *mount) {
    mount->fs   = fs;
    mount->root = tmpfs_create_dentry(0, "/", I_DIRECTORY);
    return 0;
}

struct filesystem *tmpfs = 0;
struct filesystem* tmpfs_get_filesystem() {
    if (tmpfs == 0) {
        tmpfs = (struct filesystem *)kmalloc(sizeof(struct filesystem));
        strcpy(tmpfs->name, "tmpfs");
        tmpfs->setup_mount = tmpfs_setup_mount;
    }
    return tmpfs;
}

struct inode* tmpfs_create_inode(struct dentry *dentry, unsigned int type) {
    struct inode *inode = (struct inode *)kmalloc(sizeof(struct inode));
    inode->i_op     = tmpfs_iop;
    inode->i_fop    = tmpfs_fop;
    inode->i_dentry = dentry;
    inode->i_type   = type;
    inode->i_size   = 0;
    return inode;
} 

struct dentry* tmpfs_create_dentry(struct dentry *parent, const char *name, unsigned int type) {
    struct dentry *dentry = (struct dentry *)kmalloc(sizeof(struct dentry));
    strcpy(dentry->d_name, name);
    dentry->d_parent = parent;
    dentry->d_inode  = tmpfs_create_inode(dentry, type);
    // dentry->d_op     = tmpfs_dop;
    dentry->d_mount  = 0;
    INIT_LIST_HEAD(&dentry->d_child);
    INIT_LIST_HEAD(&dentry->d_subdirs);
    if (parent) 
        list_add_tail(&dentry->d_child, &parent->d_subdirs);
    return dentry;
}