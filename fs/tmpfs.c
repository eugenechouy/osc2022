#include "fs/vfs.h"
#include "fs/tmpfs.h"
#include "kern/slab.h"
#include "string.h"

struct inode_operations *tmpfs_iop;
struct file_operations  *tmpfs_fop;
struct dentry_opartions *tmpfs_dop;

struct inode_operations *initramfs_iop;
struct file_operations  *initramfs_fop;

struct inode* tmpfs_create_inode(struct dentry *dentry, unsigned int type, unsigned int flags);
struct dentry* tmpfs_create_dentry(struct dentry *parent, const char *name, unsigned int type, unsigned int flags);

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
    struct dentry *new_dentry = tmpfs_create_dentry(dir_node->i_dentry, component_name, I_FILE, I_FRW);
    new_dentry->d_inode->internal = (void*)tmpfs_internal;
    *target = new_dentry->d_inode;
    return 0;
}

int tmpfs_mkdir(struct inode* dir_node, struct inode** target, const char* component_name) {
    struct dentry *new_dentry = tmpfs_create_dentry(dir_node->i_dentry, component_name, I_DIRECTORY, I_FRW);
    *target = new_dentry->d_inode;
    return 0;
}

int initramfs_create(struct inode* dir_node, struct inode** target, const char* component_name) {
    return -1;
}

int initramfs_mkdir(struct inode* dir_node, struct inode** target, const char* component_name) {
    return -1;
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

long tmpfs_lseek64(struct file* file, long offset, int whence) {
    unsigned long size = file->inode->i_size;
    if (whence == SEEK_SET) {
        file->f_pos = offset > size ? size : offset;
    } else if (whence == SEEK_END) {
        file->f_pos = size + offset;
    }
    return file->f_pos;
}
 
int initramfs_write(struct file *file, const void *buf, long len) {
    return 0;
}

int initramfs_read(struct file *file, void *buf, long len) {
    struct initramfs_inode *initramfs_internal = (struct initramfs_inode *)file->inode->internal;
    unsigned long size = file->inode->i_size;
    unsigned long i    = file->f_pos;
    unsigned long bi   = 0;
    char *dest = (char*)buf;
    for ( ; bi<len && i<size ; i++) {
        dest[bi] = initramfs_internal->data[i];
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
    tmpfs_fop->write   = tmpfs_write;
    tmpfs_fop->read    = tmpfs_read;
    tmpfs_fop->open    = tmpfs_open;
    tmpfs_fop->close   = tmpfs_close;
    tmpfs_fop->lseek64 = tmpfs_lseek64;
    tmpfs_dop = 0;

    initramfs_iop = (struct inode_operations *)kmalloc(sizeof(struct inode_operations));
    initramfs_iop->lookup = tmpfs_lookup;
    initramfs_iop->create = initramfs_create;
    initramfs_iop->mkdir  = initramfs_mkdir;
    initramfs_fop = (struct file_operations *)kmalloc(sizeof(struct file_operations));
    initramfs_fop->write   = initramfs_write;
    initramfs_fop->read    = initramfs_read;
    initramfs_fop->open    = tmpfs_open;
    initramfs_fop->close   = tmpfs_close;
    initramfs_fop->lseek64 = tmpfs_lseek64;
    return 0;
}

int tmpfs_setup_mount(struct filesystem *fs, struct mount *mount) {
    mount->fs   = fs;
    mount->root = tmpfs_create_dentry(0, "/", I_DIRECTORY, I_FRW);
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

struct inode* tmpfs_create_inode(struct dentry *dentry, unsigned int type, unsigned int flags) {
    struct inode *inode = (struct inode *)kmalloc(sizeof(struct inode));
    inode->i_op     = tmpfs_iop;
    inode->i_fop    = tmpfs_fop;
    inode->i_dentry = dentry;
    inode->i_flags  = flags;
    inode->i_type   = type;
    inode->i_size   = 0;
    return inode;
} 

struct dentry* tmpfs_create_dentry(struct dentry *parent, const char *name, unsigned int type, unsigned int flags) {
    struct dentry *dentry = (struct dentry *)kmalloc(sizeof(struct dentry));
    strcpy(dentry->d_name, name);
    dentry->d_parent = parent;
    dentry->d_inode  = tmpfs_create_inode(dentry, type, flags);
    // dentry->d_op     = tmpfs_dop;
    dentry->d_mount  = 0;
    INIT_LIST_HEAD(&dentry->d_child);
    INIT_LIST_HEAD(&dentry->d_subdirs);
    if (parent) 
        list_add_tail(&dentry->d_child, &parent->d_subdirs);
    return dentry;
}

#include "kern/cpio.h"

/*
    Used to create initramfs' file
    1. point directly to data
    2. RO
*/
void initramfs_init_create(struct inode *root_node, const char *pathname, char *data, int filesize) {
    struct inode *dir_node;
    struct dentry *new_dentry;
    struct initramfs_inode *initramfs_internal;
    char filename[32];

    vfs_walk_recursive(root_node, pathname, &dir_node, filename);
    initramfs_internal = (struct initramfs_inode *)kmalloc(sizeof(struct initramfs_inode));
    initramfs_internal->data = data;
    new_dentry = tmpfs_create_dentry(dir_node->i_dentry, filename, I_FILE, I_FRO);
    new_dentry->d_inode->internal = initramfs_internal;
    new_dentry->d_inode->i_size   = filesize;
    new_dentry->d_inode->i_fop    = initramfs_fop;
    new_dentry->d_inode->i_op     = initramfs_iop;
}

void initramfs_init_mkdir(struct inode *root_node, const char *pathname) {
    struct inode *dir_node;
    struct dentry *new_dentry;
    char dirname[32];

    vfs_walk_recursive(root_node, pathname, &dir_node, dirname);
    new_dentry = tmpfs_create_dentry(dir_node->i_dentry, dirname, I_DIRECTORY, I_FRO);
    new_dentry->d_inode->i_fop    = initramfs_fop;
    new_dentry->d_inode->i_op     = initramfs_iop;
}

int initramfs_setup_mount(struct filesystem *fs, struct mount *mount) {
    int i        = 0;
    int filesize = 0;
    int namesize = 0;
    int mode     = 0;
    struct cpio_newc_header *header;
    
    mount->fs = fs;
    mount->root = tmpfs_create_dentry(0, "/", I_DIRECTORY, I_FRO);
    mount->root->d_inode->i_fop = initramfs_fop;
    mount->root->d_inode->i_op  = initramfs_iop;
    
    for ( ; ; i+=namesize+filesize) {
        header = ((struct cpio_newc_header *)(CPIO_ADDRESS + i));
        if (strncmp(header->c_magic, CPIO_MAGIC, 6)) {
            break;
        }
        filesize = (atoi(header->c_filesize, 16, 8) + 3) & -4;
        namesize = ((atoi(header->c_namesize, 16, 8) + 6 + 3) & -4) - 6;
        i += sizeof(struct cpio_newc_header);
        if (!strncmp((char *)(CPIO_ADDRESS + i), CPIO_END, 10))
            break;
        if (!strncmp((char *)(CPIO_ADDRESS + i), ".", 1)) {
            continue;
        }
        mode = atoi(header->c_mode, 16, 8);
        if ((mode & 00170000) == 0100000) {
            initramfs_init_create(mount->root->d_inode, (char *)(CPIO_ADDRESS + i), (char *)(CPIO_ADDRESS + i + namesize), atoi(header->c_filesize, 16, 8));
        }
        if ((mode & 00170000) == 0040000) {
            initramfs_init_mkdir(mount->root->d_inode, (char *)(CPIO_ADDRESS + i));
        }
    }

    return 0;
}

struct filesystem *initramfs = 0;
struct filesystem* initramfs_get_filesystem() {
    if (initramfs == 0) {
        initramfs = (struct filesystem *)kmalloc(sizeof(struct filesystem));
        strcpy(initramfs->name, "initramfs");
        initramfs->setup_mount = initramfs_setup_mount;
    }
    return initramfs;
}
