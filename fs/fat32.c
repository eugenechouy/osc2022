#include "fs/vfs.h"
#include "fs/fat32.h"
#include "kern/slab.h"
#include "kern/kio.h"
#include "kern/pagecache.h"
#include "string.h"

#define MIN(a,b) ((a) > (b) ? (b) : (a))

struct fat32_info fat32_info = {0};

#define clus_2_lba(cluster) (fat32_info.data_lba + (cluster - fat32_info.root_clus) * fat32_info.clus_size)
#define fat_lba(cluster)    (fat32_info.fat_lba + (cluster / FAT32_ENTRY_PER_SECT))
#define fat_idx(cluster)    (cluster / FAT32_ENTRY_PER_SECT)

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

void to_sfn(unsigned char *name, unsigned char *ext, const char *source) {
    int i;
    int t = 0;

    i = 0;
    while(source[t] != '\0' && i < 8) {
        if (source[t] == '.') {
            t++;
            break;
        }
        name[i++] = (unsigned char)source[t];
        t++;
    }
    while (i < 8) name[i++] = ' ';

    i = 0;
    while(source[t] != '\0' && i < 3) {
        ext[i++] = (unsigned char)source[t];
        t++;
    }
    while (i < 3) ext[i++] = ' ';
}

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

/*
    1. Find an empty entry in the FAT table.
    2. Find an empty directory entry in the target directory.
*/
int fat32_create(struct inode* dir_node, struct inode** target, const char* component_name) {
    char         filename[13];
    unsigned int *fat;
    unsigned int i;
    unsigned int file_first_clus = 1;
    unsigned int unused_entry;
    unsigned int clus = fat32_info.fsi_next_free;
    struct fat32_dir_entry *dentry_list;
    struct fat32_inode *fat32_internal = (struct fat32_inode*)dir_node->internal;

    struct dentry *new_dentry;

    // Find an empty entry in the FAT table.
    while(file_first_clus == 1) {
        fat = pagecache_read(fat_lba(clus));
        for (i=(clus%FAT32_ENTRY_PER_SECT) ; i<FAT32_ENTRY_PER_SECT ; i++, clus++) {
            if (fat[i] == FAT32_ENTRY_FREE) {
                fat[i] = FAT32_ENTRY_ALLOCATED_AND_END_OF_FILE;
                file_first_clus = (clus/FAT32_ENTRY_PER_SECT) * FAT32_ENTRY_PER_SECT  + i;
                kprintf("fat free %d\n", file_first_clus);
                break;
            }
        }
    }
    pagecache_dirty(fat_lba(clus));
    
    // Find an empty directory entry in the target directory.
    dentry_list = (struct fat32_dir_entry *)pagecache_read(clus_2_lba(fat32_internal->cluster));
    unused_entry = -1;
    for(i=0 ; dentry_list[i].DIR_Name[0]!=FAT32_DIR_ENTRY_LAST_AND_UNUSED && i<FAT32_DIR_ENTRY_PRT_SEC ; i++) {
        if (dentry_list[i].DIR_Name[0] == FAT32_DIR_ENTRY_UNUSED && unused_entry == -1) {
            unused_entry = i;
            continue;
        }
        
        if ((dentry_list[i].DIR_Attr & FAT32_DIR_ENTRY_ATTR_LONG_NAME_MASK) == FAT32_DIR_ENTRY_ATTR_LONG_NAME) {
            continue;
        } else
            get_sfn(dentry_list[i].DIR_Name, dentry_list[i].DIR_Ext, filename);

        if (!strcmp(filename, component_name)) {
            kprintf("fat32_create: %s already existed\n", component_name);
            return -2;
        }
    }
    if (unused_entry == -1) {
        if (i+1 == FAT32_DIR_ENTRY_PRT_SEC) {
            kprintf("fat32_create: directory entry fulled!\n");
            return -3;
        }
        dentry_list[i+1].DIR_Name[0] = FAT32_DIR_ENTRY_LAST_AND_UNUSED;
        unused_entry = i;
    }
    to_sfn(dentry_list[unused_entry].DIR_Name, dentry_list[unused_entry].DIR_Ext, component_name);
    dentry_list[unused_entry].DIR_Attr = FAT32_DIR_ENTRY_ATTR_ARCHIVE;
    dentry_list[unused_entry].DIR_FstClusHI = file_first_clus & 0xffff0000;
    dentry_list[unused_entry].DIR_FstClusLO = file_first_clus & 0x0000ffff;
    dentry_list[unused_entry].DIR_FileSize  = 0;
    pagecache_dirty(clus_2_lba(fat32_internal->cluster));

    // in-memory inode tree
    fat32_internal = (struct fat32_inode*)kmalloc(sizeof(struct fat32_inode));
    fat32_internal->cluster = file_first_clus;
    new_dentry = fat32_create_dentry(dir_node->i_dentry, component_name, I_FILE, I_FRW);
    new_dentry->d_inode->internal = fat32_internal;
    *target = new_dentry->d_inode;

    return -0;
}

int fat32_mkdir(struct inode* dir_node, struct inode** target, const char* component_name) {
    return -1;
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
    char filename[13];
    struct dentry          *new_dentry;
    struct fat32_dir_entry *dentry_list;
    struct fat32_inode     *fat32_internal = (struct fat32_inode*)dentry->d_inode->internal;
    
    dentry_list = (struct fat32_dir_entry *)pagecache_read(clus_2_lba(fat32_internal->cluster));
    for(i=0 ; dentry_list[i].DIR_Name[0]!=FAT32_DIR_ENTRY_LAST_AND_UNUSED && i<FAT32_DIR_ENTRY_PRT_SEC ; i++) {
        if (dentry_list[i].DIR_Name[0] == FAT32_DIR_ENTRY_UNUSED)
            continue;
        
        if ((dentry_list[i].DIR_Attr & FAT32_DIR_ENTRY_ATTR_LONG_NAME_MASK) == FAT32_DIR_ENTRY_ATTR_LONG_NAME) {
            kprintf("LFN\n");
            continue;
        } else
            get_sfn(dentry_list[i].DIR_Name, dentry_list[i].DIR_Ext, filename);

        if (dentry_list[i].DIR_Attr == FAT32_DIR_ENTRY_ATTR_DIRECTORY) {
            new_dentry = fat32_create_dentry(dentry, filename, I_DIRECTORY, I_FRW);
            kprintf("Dir: %s\n", filename);
        } else {
            new_dentry = fat32_create_dentry(dentry, filename, I_FILE, I_FRW);
            kprintf("File: %s\n", filename);
        }
        
        fat32_internal = (struct fat32_inode*)kmalloc(sizeof(struct fat32_inode));
        fat32_internal->cluster  = ((dentry_list[i].DIR_FstClusHI << 16) | (dentry_list[i].DIR_FstClusLO));
        new_dentry->d_inode->i_size   = dentry_list[i].DIR_FileSize;
        new_dentry->d_inode->internal = fat32_internal;
    }
    dentry->d_cached = 1;
    return 0;
}

int fat32_update_dentry(struct dentry *dentry, const char *component_name, unsigned int filesize) {
    int i;
    char filename[13];
    struct fat32_dir_entry *dentry_list;
    struct fat32_inode     *fat32_internal = (struct fat32_inode*)dentry->d_inode->internal;

    dentry_list = (struct fat32_dir_entry *)pagecache_read(clus_2_lba(fat32_internal->cluster));
    for(i=0 ; dentry_list[i].DIR_Name[0]!=FAT32_DIR_ENTRY_LAST_AND_UNUSED && i<FAT32_DIR_ENTRY_PRT_SEC ; i++) {
        if (dentry_list[i].DIR_Name[0] == FAT32_DIR_ENTRY_UNUSED)
            continue;

        if ((dentry_list[i].DIR_Attr & FAT32_DIR_ENTRY_ATTR_LONG_NAME_MASK) == FAT32_DIR_ENTRY_ATTR_LONG_NAME) {
            kprintf("LFN\n");
            continue;
        } else
            get_sfn(dentry_list[i].DIR_Name, dentry_list[i].DIR_Ext, filename);

        if (!strcmp(filename, component_name)) {
            kprintf("extend file %d -> %d\n", dentry_list[i].DIR_FileSize, filesize);
            dentry_list[i].DIR_FileSize = filesize;
            pagecache_dirty(clus_2_lba(fat32_internal->cluster));
            return 0;
        }
    }
    kprintf("dentry lookup not found\n");
    return -1;
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
    struct fat32_inode *fat32_internal = (struct fat32_inode*)file->inode->internal;
    unsigned long bi = 0;
    unsigned long j;
    unsigned long offset;
    unsigned long end;
    unsigned int cur_clus;
    unsigned char *buffer;
    char *src = (char*)buf;

    if (file->cur_clus == 1)
        cur_clus = fat32_internal->cluster;
    else
        cur_clus = file->cur_clus;

    while(len > 0 && cur_clus < FAT32_ENTRY_RESERVED_TO_END) {
        buffer = pagecache_read(clus_2_lba(cur_clus));
        // data range in this block
        offset = file->f_pos % BLOCK_SIZE;
        end    = MIN(BLOCK_SIZE, offset + len);
        for(j=offset ; j<end ; j++) {
            buffer[j] = src[bi];
            bi++;
        }
        pagecache_dirty(clus_2_lba(cur_clus));
        file->f_pos += (end - offset);
        len         -= (end - offset);
        if (len) {
            // searching an FAT table 
            kprintf("fat32_write: file size exceed 512 bytes! not supported yet\n");
            break;
        }
    }
    file->cur_clus = cur_clus;
    if (file->f_pos > file->inode->i_size) {
        file->inode->i_size = file->f_pos;
        fat32_update_dentry(file->inode->i_dentry->d_parent, file->inode->i_dentry->d_name, file->inode->i_size);
    }
    return bi;
}

int fat32_read(struct file *file, void *buf, long len) {
    struct fat32_inode *fat32_internal = (struct fat32_inode*)file->inode->internal;
    unsigned long size    = file->inode->i_size;
    unsigned long i       = 0;
    unsigned long bi      = 0;
    unsigned long j;
    unsigned long offset;
    unsigned long end;
    unsigned int cur_clus;
    unsigned int *fat;
    unsigned char *buffer;
    char *dest = (char*)buf;

    if (file->cur_clus == 1)
        cur_clus = fat32_internal->cluster;
    else
        cur_clus = file->cur_clus;

    while(len > 0 && i < size && cur_clus < FAT32_ENTRY_RESERVED_TO_END) {
        buffer = pagecache_read(clus_2_lba(cur_clus));
        // data range in this block
        offset = file->f_pos % BLOCK_SIZE;
        end    = MIN(BLOCK_SIZE, offset + len);
        if (i + (end-offset) > size) {
            end = size % BLOCK_SIZE;
            len = 0;
        } else 
            len -= (end - offset);
        for(j=offset ; j<end ; j++) {
            dest[bi] = buffer[j];
            bi++;
        }
        file->f_pos += (end - offset);
        i           += BLOCK_SIZE;
        if (len) {
            // lookup FAT
            fat = pagecache_read(fat_lba(cur_clus));
            cur_clus = fat[cur_clus % FAT32_ENTRY_PER_SECT];
        }
    }
    file->cur_clus = cur_clus;

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
    struct fat32_inode *fat32_internal = (struct fat32_inode*)kmalloc(sizeof(struct fat32_inode));
    
    mount->fs   = fs;
    mount->root = fat32_create_dentry(0, "/", I_DIRECTORY, I_FRW);

    if (fat32_info.root_clus != 2)
        kprintf("Warning: fat32 root cluster number differed.\n");
    
    fat32_internal->cluster  = fat32_info.root_clus;
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