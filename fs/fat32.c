#include "fs/vfs.h"
#include "fs/fat32.h"
#include "kern/slab.h"
#include "string.h"

#define MIN(a,b) ((a) > (b) ? (b) : (a))

struct fat32_info fat32_info = {0};

#define clus_2_lba(cluster) (fat32_info.data_lba + (cluster - fat32_info.root_clus) * fat32_info.clus_size)
#define fat_lba(cluster)    (fat32_info.fat_lba + (cluster / FAT32_ENTRY_PER_SECT))

struct inode* fat32_create_inode(struct dentry *dentry, unsigned int type, unsigned int flags);
struct dentry* fat32_create_dentry(struct dentry *parent, const char *name, unsigned int type, unsigned int flags);

/*
    inode operation
*/
int fat32_lookup(struct inode* dir_node, struct inode** target, const char* component_name) {
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

int fat32_create(struct inode* dir_node, struct inode** target, const char* component_name) {
    return -1;
}

int fat32_mkdir(struct inode* dir_node, struct inode** target, const char* component_name) {
    return -1;
}

void get_sfn(unsigned char *name, unsigned char *ext, char *target) {
    int i;
    int t = 0;
    char c;
    for(i=0 ; i<8 ; i++) {
        c = name[i];
        if (c == ' ')
            break;
        target[t++] = c;
    }
    target[t++] = '.';
    for(i=0 ; i<3 ; i++) {
        c = ext[i];
        if (c == ' ')
            break;
        target[t++] = c;
    }
    target[t++] = '\0';
}

struct inode_operations fat32_iop = {
    .lookup = fat32_lookup,
    .create = fat32_create,
    .mkdir  = fat32_mkdir,
};


/*
    dentry operation
*/
int fat32_read_dentry(struct dentry *dentry) {
    int i;
    char          filename[13];
    unsigned char buffer[BLOCK_SIZE];
    struct dentry          *new_dentry;
    struct fat32_dir_entry *dentry_list;
    struct fat32_internal  *fat32_internal = (struct fat32_internal*)dentry->d_inode->internal;
    
    readblock(clus_2_lba(fat32_internal->cluster), buffer);

    dentry_list = (struct fat32_dir_entry *)buffer;

    for(i=0 ; dentry_list[i].DIR_Name[0]!=FAT32_DIR_ENTRY_LAST_AND_UNUSED ; i++) {
        if (dentry_list[i].DIR_Name[0] == FAT32_DIR_ENTRY_UNUSED)
            continue;
        
        if (dentry_list[i].DIR_Attr & FAT32_DIR_ENTRY_ATTR_LONG_NAME_MASK == FAT32_DIR_ENTRY_ATTR_LONG_NAME) {
            // kprintf("LFN\n");
            continue;
        } else
            get_sfn(dentry_list[i].DIR_Name, dentry_list[i].DIR_Ext, filename);


        if (dentry_list[i].DIR_Attr == FAT32_DIR_ENTRY_ATTR_DIRECTORY) {
            new_dentry = fat32_create_dentry(dentry, filename, I_DIRECTORY, I_FRW);
            // kprintf("Dir: %s\n", filename);
        } else {
            new_dentry = fat32_create_dentry(dentry, filename, I_FILE, I_FRW);
            // kprintf("File: %s\n", filename);
        }
        
        fat32_internal = (struct fat32_internal*)kmalloc(sizeof(struct fat32_internal));
        fat32_internal->cluster  = ((dentry_list[i].DIR_FstClusHI << 16) | (dentry_list[i].DIR_FstClusLO));
        fat32_internal->cur_clus = fat32_internal->cluster;
        new_dentry->d_inode->i_size   = dentry_list[i].DIR_FileSize;
        new_dentry->d_inode->internal = fat32_internal;
    }
    dentry->d_cached = 1;

    return 0;
}

struct dentry_operations fat32_dop = {
    .read = fat32_read_dentry,
};


/*
    file operation
*/
int fat32_open(struct inode* file_node, struct file** target) {
    return 0;
}

int fat32_close(struct file *file) {
    return 0;
}

int fat32_write(struct file *file, const void *buf, long len) {
    return -1;
}

int fat32_read(struct file *file, void *buf, long len) {
    struct fat32_internal *fat32_internal = (struct fat32_internal*)file->inode->internal;
    unsigned long size    = file->inode->i_size;
    unsigned long i       = 0;
    unsigned long bi      = 0;
    unsigned long j;
    unsigned long offset;
    unsigned long end;
    unsigned int *fat;
    unsigned char buffer[BLOCK_SIZE];
    char *dest = (char*)buf;

    while(len > 0 && i < size && fat32_internal->cur_clus < FAT32_ENTRY_RESERVED_TO_END) {
        readblock(clus_2_lba(fat32_internal->cur_clus), buffer);
        // data range in this block
        offset = file->f_pos % BLOCK_SIZE;
        end    = MIN(BLOCK_SIZE, offset + len);
        if (i + (end-offset) > size)
            end = size % BLOCK_SIZE;
        for(j=offset ; j<end ; j++) {
            dest[bi] = buffer[j];
            bi++;
        }
        file->f_pos += (end - offset);
        len         -= (end - offset);
        i           += BLOCK_SIZE;
        if (len) {
            // lookup FAT
            readblock(fat_lba(fat32_internal->cur_clus), buffer);
            fat = (unsigned int *)buffer;
            fat32_internal->cur_clus = fat[fat32_internal->cur_clus % FAT32_ENTRY_PER_SECT];
        }
    }

    return bi;
}

long fat32_lseek64(struct file* file, long offset, int whence) {
    return -1;
}

struct file_operations fat32_fop = {
    .write   = fat32_write,
    .read    = fat32_read,
    .open    = fat32_open,
    .close   = fat32_close,
    .lseek64 = fat32_lseek64,
};


int fat32_setup_mount(struct filesystem *fs, struct mount *mount) {
    struct fat32_internal *fat32_internal = (struct fat32_internal*)kmalloc(sizeof(struct fat32_internal));
    
    mount->fs   = fs;
    mount->root = fat32_create_dentry(0, "/", I_DIRECTORY, I_FRW);

    if (fat32_info.root_clus != 2)
        kprintf("Warning: fat32 root cluster number differed.\n");
    
    fat32_internal->cluster  = fat32_info.root_clus;
    fat32_internal->cur_clus = fat32_internal->cluster;
    mount->root->d_inode->internal = fat32_internal;
    return 0;
}

struct filesystem *fat32 = 0;
struct filesystem* fat32_get_filesystem() {
    if (fat32 == 0) {
        fat32 = (struct filesystem *)kmalloc(sizeof(struct filesystem));
        strcpy(fat32->name, "fat32");
        fat32->setup_mount = fat32_setup_mount;
    }
    return fat32;
}

struct inode* fat32_create_inode(struct dentry *dentry, unsigned int type, unsigned int flags) {
    struct inode *inode = (struct inode *)kmalloc(sizeof(struct inode));
    inode->i_op     = &fat32_iop; 
    inode->i_fop    = &fat32_fop;
    inode->i_dentry = dentry;
    inode->i_flags  = flags;
    inode->i_type   = type;
    inode->i_size   = 0;
    return inode;
}

struct dentry* fat32_create_dentry(struct dentry *parent, const char *name, unsigned int type, unsigned int flags) {
    struct dentry *dentry = (struct dentry *)kmalloc(sizeof(struct dentry));
    strcpy(dentry->d_name, name);
    dentry->d_cached = 0;
    dentry->d_parent = parent;
    dentry->d_inode  = fat32_create_inode(dentry, type, flags);
    dentry->d_op     = &fat32_dop;
    dentry->d_mount  = 0;
    INIT_LIST_HEAD(&dentry->d_child);
    INIT_LIST_HEAD(&dentry->d_subdirs);
    if (parent)
        list_add_tail(&dentry->d_child, &parent->d_subdirs);
    return dentry;
}