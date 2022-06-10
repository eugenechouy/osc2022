#ifndef MBR_H
#define MBR_H

struct chs {
    unsigned char cylinder;
    unsigned char head;
    unsigned char sector;
};

/*
    MBR structure
    000 ~ 1BD  446  Code Area
    1BE ~ 1FD  64   Master Partition Table
        1BE ~ 1CD  16  Table entry for primary partition #1
        1CE ~ 1DD  16  Table entry for primary partition #2
        1DE ~ 1ED  16  Table entry for primary partition #3
        1EE ~ 1FD  16  Table entry for primary partition #4
    1FE ~ 1FF  2    Boot Record Signature

    Partition Types
    0x00 Empty partition entry
    0x07 NTFS
    0x0B FAT32 with CHS (Cylinder, Head, Sector) Addressing
    0x0C FAT32 with LBA
    0x0F Extended Partition with LBA
    0x82 Linux Swap Space
    0x83 Linux File System
*/
struct mbr {
    unsigned char code[446];
    struct mbr_partition {
        unsigned char status;
        struct chs first_sector;
        unsigned char partition_type;
        struct chs last_sector;
        unsigned int first_sector_lba;
        unsigned int sectors;
    } partition[4];
    unsigned short mbr_signature;
} __attribute__((packed));

#define MBR_SIGNATURE         0xAA55
#define MBR_STATUS_BOOTABLE   0x80


#endif