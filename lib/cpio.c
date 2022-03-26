#include "peripheral/uart.h"
#include "string.h"
#include "cpio.h"
#include "byteswap.h"

void *CPIO_ADDRESS = 0x8000000;

void initramfs_callback(char *node_name, char *prop_name, void *prop_value) {
    if (!strncmp(node_name, "chosen", 6) && !strncmp(prop_name, "linux,initrd-start", 18)) {
        uart_puts("cpio: Find!\n");
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
            uart_puts("cpio: Bad magic\n");
            break;
        }
        filesize = (atoi(header->c_filesize, 16, 8) + 3) & -4;
        namesize = ((atoi(header->c_namesize, 16, 8) + 6 + 3) & -4) - 6;
        i += sizeof(struct cpio_newc_header);
        if (!strncmp((char *)(CPIO_ADDRESS + i), CPIO_END, 10))
            break;
        uart_puts((char *)(CPIO_ADDRESS + i));
        uart_puts("\n");
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
            uart_puts("cpio: Bad magic\n");
            break;
        }
        filesize = (atoi(header->c_filesize, 16, 8) + 3) & -4;
        namesize = ((atoi(header->c_namesize, 16, 8) + 6 + 3) & -4) - 6;
        i += sizeof(struct cpio_newc_header);
        if (!strcmp((char *)(CPIO_ADDRESS + i), filename)) {
            uart_puts((char *)(CPIO_ADDRESS + i + namesize));
            uart_puts("\n");
            return;
        }
        if (!strncmp((char *)(CPIO_ADDRESS + i), CPIO_END, 10))
            break;
    }
    uart_puts("File not exists...\n");
}

void cpio_exec(const char *filename) {
    int i        = 0;
    int filesize = 0;
    int namesize = 0;
    struct cpio_newc_header *header;
    char *target;

    for ( ; ; i+=namesize+filesize) {
        header = ((struct cpio_newc_header *)(CPIO_ADDRESS + i));
        if (strncmp(header->c_magic, CPIO_MAGIC, 6)) {
            uart_puts("cpio: Bad magic\n");
            break;
        }
        filesize = (atoi(header->c_filesize, 16, 8) + 3) & -4;
        namesize = ((atoi(header->c_namesize, 16, 8) + 6 + 3) & -4) - 6;
        i += sizeof(struct cpio_newc_header);
        if (!strcmp((char *)(CPIO_ADDRESS + i), filename))
            break;
        if (!strncmp((char *)(CPIO_ADDRESS + i), CPIO_END, 10)) {
            uart_puts("File not exists...\n");
            return;
        }
    }
    i += namesize;

    from_el1_to_el0(CPIO_ADDRESS + i);
}
