#include "kern/kio.h"
#include "kern/mm.h"
#include "kern/cpio.h"
#include "kern/sched.h"
#include "string.h"
#include "byteswap.h"

void *CPIO_ADDRESS = (void*)0x8000000;
void *CPIO_END_ADR = 0;

void initramfs_callback(char *node_name, char *prop_name, void *prop_value) {
    if (!strncmp(node_name, "chosen", 6) && !strncmp(prop_name, "linux,initrd-start", 18)) {
        kputs("cpio: Find!\n");
        CPIO_ADDRESS = (void*)__bswap_32(*((unsigned int *)(prop_value)));
    }
}

void cpio_ls() {
    int i        = 0;
    int filesize = 0;
    int namesize = 0;
    struct cpio_newc_header *header;

    for ( ; ; i+=namesize+filesize) {
        header = ((struct cpio_newc_header *)(CPIO_ADDRESS + i));
        if (strncmp(header->c_magic, CPIO_MAGIC, 6)) {
            kputs("cpio: Bad magic\n");
            break;
        }
        filesize = (atoi(header->c_filesize, 16, 8) + 3) & -4;
        namesize = ((atoi(header->c_namesize, 16, 8) + 6 + 3) & -4) - 6;
        i += sizeof(struct cpio_newc_header);
        if (!strncmp((char *)(CPIO_ADDRESS + i), CPIO_END, 10))
            break;
        kprintf("%s\n", (char *)(CPIO_ADDRESS + i));
    }
}

void cpio_cat(const char *filename) {
    int i        = 0;
    int filesize = 0;
    int namesize = 0;
    struct cpio_newc_header *header;

    for ( ; ; i+=namesize+filesize) {
        header = ((struct cpio_newc_header *)(CPIO_ADDRESS + i));
        if (strncmp(header->c_magic, CPIO_MAGIC, 6)) {
            kputs("cpio: Bad magic\n");
            break;
        }
        filesize = (atoi(header->c_filesize, 16, 8) + 3) & -4;
        namesize = ((atoi(header->c_namesize, 16, 8) + 6 + 3) & -4) - 6;
        i += sizeof(struct cpio_newc_header);
        if (!strcmp((char *)(CPIO_ADDRESS + i), filename)) {
            kprintf("%s\n", (char *)(CPIO_ADDRESS + i + namesize));
            return;
        }
        if (!strncmp((char *)(CPIO_ADDRESS + i), CPIO_END, 10))
            break;
    }
    kputs("File not exists...\n");
}

extern void from_el1_to_el0(void *);

void cpio_exec(const char *filename) {
    int i        = 0;
    int filesize = 0;
    int namesize = 0;
    struct cpio_newc_header *header;

    for ( ; ; i+=namesize+filesize) {
        header = ((struct cpio_newc_header *)(CPIO_ADDRESS + i));
        if (strncmp(header->c_magic, CPIO_MAGIC, 6)) {
            kputs("cpio: Bad magic\n");
            break;
        }
        filesize = (atoi(header->c_filesize, 16, 8) + 3) & -4;
        namesize = ((atoi(header->c_namesize, 16, 8) + 6 + 3) & -4) - 6;
        i += sizeof(struct cpio_newc_header);
        if (!strcmp((char *)(CPIO_ADDRESS + i), filename))
            break;
        if (!strncmp((char *)(CPIO_ADDRESS + i), CPIO_END, 10)) {
            kputs("File not exists...\n");
            return;
        }
    }
    i += namesize;

    do_exec(CPIO_ADDRESS + i);
}

char* cpio_find(const char *filename) {
    int i        = 0;
    int filesize = 0;
    int namesize = 0;
    struct cpio_newc_header *header;

    for ( ; ; i+=namesize+filesize) {
        header = ((struct cpio_newc_header *)(CPIO_ADDRESS + i));
        if (strncmp(header->c_magic, CPIO_MAGIC, 6)) {
            kputs("cpio: Bad magic\n");
            break;
        }
        filesize = (atoi(header->c_filesize, 16, 8) + 3) & -4;
        namesize = ((atoi(header->c_namesize, 16, 8) + 6 + 3) & -4) - 6;
        i += sizeof(struct cpio_newc_header);
        if (!strcmp((char *)(CPIO_ADDRESS + i), filename))
            break;
        if (!strncmp((char *)(CPIO_ADDRESS + i), CPIO_END, 10)) {
            kputs("File not exists...\n");
            return 0;
        }
    }
    i += namesize;

    return CPIO_ADDRESS + i;
}

void cpio_reserve() {
    int i        = 0;
    int filesize = 0;
    int namesize = 0;
    struct cpio_newc_header *header;

    for ( ; ; i+=namesize+filesize) {
        header = ((struct cpio_newc_header *)(CPIO_ADDRESS + i));
        if (strncmp(header->c_magic, CPIO_MAGIC, 6)) {
            kputs("cpio: Bad magic\n");
            break;
        }
        filesize = (atoi(header->c_filesize, 16, 8) + 3) & -4;
        namesize = ((atoi(header->c_namesize, 16, 8) + 6 + 3) & -4) - 6;
        i += sizeof(struct cpio_newc_header);
        if (!strncmp((char *)(CPIO_ADDRESS + i), CPIO_END, 10))
            break;
    }
    mm_reserve(CPIO_ADDRESS, CPIO_ADDRESS+i);
}