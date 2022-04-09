#ifndef DTB_H
#define DTB_H

// #define FDT_HEADER_MAGIC    0xd00dfeed
#define FDT_BEGIN_NODE      0x00000001
#define FDT_END_NODE        0x00000002
#define FDT_PROP            0x00000003
#define FDT_NOP             0x00000004
#define FDT_END             0x00000009

// All the header fields are stored in big-endian format
struct fdt_header { 
    unsigned int magic; 
    unsigned int totalsize;
    unsigned int off_dt_struct;
    unsigned int off_dt_strings;
    unsigned int off_mem_rsvmap;
    unsigned int version;
    unsigned int last_comp_version;
    unsigned int boot_cpuid_phys;
    unsigned int size_dt_strings;
    unsigned int size_dt_struct;
};

struct fdt_reserve_entry {
    unsigned long long address;
    unsigned long long size;
};

struct fdt_prop {
    unsigned int len;
    unsigned int nameoff;
};

int fdt_init();
int fdt_traverse(void (*cb)(char *, char *, void *));

#endif