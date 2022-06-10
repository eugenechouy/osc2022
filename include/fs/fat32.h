#ifndef FAT32_H
#define FAT32_H

#include "drivers/sdhost.h"

struct fat32_boot_sector {
    char BS_jmpBoot[3];
    char BS_OEMName[8];
    unsigned short BPB_BytsPerSec;
    unsigned char  BPB_SecPerClus;
    unsigned short BPB_RsvdSecCnt;
    unsigned char  BPB_NumFATs;
    unsigned short BPB_RootEntCnt; // Maximum number of FAT12 or FAT16 root directory entries. 0 for FAT32, where the root directory is stored in ordinary data clusters
    unsigned short BPB_TotSec16;
    char           BPB_Media;
    unsigned short BPB_FATSz16;

    unsigned short BPB_SecPerTrk;
    unsigned short NumHeads;
    unsigned int   BPB_HiddSec;
    unsigned int   BPB_TotSec32;

    unsigned int   BPB_FATSz32;
    unsigned short BPB_flags;
    unsigned short BPB_FSVer;
    unsigned int   BPB_RootClus; // Cluster number of root directory start, typically 2 (first cluster[37]) if it contains no bad sector. 
    unsigned short BPB_FSInfo;
    unsigned short BPB_BkBootSec;
    char           BPB_Reserved[12];
    unsigned char  BS_DrvNum;
    unsigned char  BS_Reserved1;
    char           BS_BootSig;
    unsigned int   BS_VolID;
    char BS_VolLAB[11];
    char BS_FilSysType[8];
    char code[420];
    char bootsig[2];
} __attribute__((packed));

// in-memory cache for important metadata
struct fat32_info {
    unsigned short sec_size;
    unsigned char  clus_size; // number of sector

    unsigned int  first_lba;
    unsigned int  fat_lba;
    unsigned char fat_num;
    unsigned int  fat_size;
    unsigned int  data_lba;

    unsigned int  root_clus;
};

struct fat32_internal {
    unsigned int cluster;
};

// file allocation table
#define FAT32_ENTRY_PER_SECT                  (BLOCK_SIZE / sizeof(unsigned int))
#define FAT32_ENTRY_FREE                      0
#define FAT32_ENTRY_RESERVED_TO_END           0x0ffffff8
#define FAT32_ENTRY_DEFECTIVE_CLUSTER         0x0ffffff7
#define FAT32_ENTRY_ALLOCATED_AND_END_OF_FILE 0x0fffffff


#define FAT32_DIR_ENTRY_LAST_AND_UNUSED 0x0
#define FAT32_DIR_ENTRY_UNUSED          0xE5

#define FAT32_DIR_ENTRY_ATTR_READ_ONLY  0x01
#define FAT32_DIR_ENTRY_ATTR_HIDDEN     0x02
#define FAT32_DIR_ENTRY_ATTR_SYSTEM     0x04
#define FAT32_DIR_ENTRY_ATTR_VOLUME_ID  0x08
#define FAT32_DIR_ENTRY_ATTR_DIRECTORY  0x10
#define FAT32_DIR_ENTRY_ATTR_ARCHIVE    0x20
#define FAT32_DIR_ENTRY_ATTR_LONG_NAME ( FAT32_DIR_ENTRY_ATTR_READ_ONLY | \
                                         FAT32_DIR_ENTRY_ATTR_HIDDEN | \
                                         FAT32_DIR_ENTRY_ATTR_SYSTEM | \
                                         FAT32_DIR_ENTRY_ATTR_VOLUME_ID )
#define FAT32_DIR_ENTRY_ATTR_LONG_NAME_MASK 0b111111
                                         

struct fat32_dir_entry {
    unsigned char   DIR_Name[8];     /* Offset 0 */     
    unsigned char   DIR_Ext[3];
    unsigned char   DIR_Attr;        /* Offset 11 */
    unsigned char   DIR_NTRes;       /* Offset 12, Reserved */
    unsigned char   DIR_CTimeHundth; /* Creation time, centiseconds (0-199) */
    unsigned short  DIR_CTime;       /* Creation time */
    unsigned short  DIR_CDate;       /* Creation date */
    unsigned short  DIR_LstAccDate;  /* Offset 18, Last access date */
    unsigned short  DIR_FstClusHI;   /* First cluster number high two bytes */
    unsigned short  DIR_WrtTime;     /* Write Time */
    unsigned short  DIR_WrtDate;     /* Write Date */
    unsigned short  DIR_FstClusLO;   /* First cluster number low two bytes */
    unsigned int    DIR_FileSize;    /* file size (in bytes) */
} __attribute__((packed));

struct fat32_dir_long_entry {
    unsigned char    id;             /* sequence number for slot */
    unsigned char    name0_4[10];    /* first 5 characters in name */
    unsigned char    attr;           /* attribute byte */
    unsigned char    reserved;       /* always 0 */
    unsigned char    alias_checksum; /* checksum for 8.3 alias */
    unsigned char    name5_10[12];   /* 6 more characters in name */
    unsigned short   start;         /* starting cluster number, 0 in long slots */
    unsigned char    name11_12[4];   /* last 2 characters in name */
} __attribute__((packed));


extern struct fat32_info fat32_info;

struct filesystem* fat32_get_filesystem();

#endif