#ifndef MMU_H
#define MMU_H


#define KERNEL_VA_BASE 0xffff000000000000

/*
    TCR
*/
#define TCR_CONFIG_REGION_48bit     (((64 - 48) << 0) | ((64 - 48) << 16))
#define TCR_CONFIG_4KB              ((0b00 << 14) |  (0b10 << 30))
#define TCR_CONFIG_DEFAULT          (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)

/*
    MAIR
    Device memory                0b0000dd00
    Normal memory                0booooiiii, (oooo != 0000 and iiii != 0000)
        oooo == 0b0100           Outer Non-cacheable (L2)
        iiii == 0b0100           Inner Non-cacheable (L1)
*/
#define MAIR_DEVICE_nGnRnE      0b00000000
#define MAIR_NORMAL_NOCACHE     0b01000100
#define MAIR_IDX_DEVICE_nGnRnE  0
#define MAIR_IDX_NORMAL_NOCACHE 1
#define MAIR_CONFIG_DEFAULT     ((MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8)) | (MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL_NOCACHE * 8)))

/*
    Page’s Descriptor
*/
#define PD_TABLE 0b11
#define PD_BLOCK 0b01
#define PD_PAGE  0b11 

#define PD_USER_RW      (0b01 << 6)

#define PD_ACCESS       (1 << 10)

#define PGD0_ATTR               PD_TABLE
#define PUD0_ATTR               PD_TABLE
#define PUD1_ATTR               (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK)
#define PMD0_ATTR               PD_TABLE
#define PTE_DEVICE_ATTR         (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_PAGE)
#define PTE_NORMAL_ATTR         (PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_PAGE)
#define PTE_NORMAL_LAZY_ATTR    ((MAIR_IDX_NORMAL_NOCACHE << 2) | PD_PAGE)

#endif