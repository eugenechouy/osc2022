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

void cpio_parse() {
    int i;
    int filesize = 0;
    int namesize = 0;
    struct cpio_newc_header *header;

    for (i=0 ; ; i+=namesize) {
        header = ((struct cpio_newc_header *)(CPIO_ADDRESS + i));
        if (strncmp(header->c_magic, CPIO_MAGIC, 6)) {
            uart_puts("cpio: Bad magic\n");
            break;
        }
        filesize = atoi(header->c_filesize, 16, 8);
        namesize = atoi(header->c_namesize, 16, 8);

        i += sizeof(struct cpio_newc_header);
        if (!strncmp((char *)(CPIO_ADDRESS + i), CPIO_END, 10))
            break;
        uart_puts((char *)(CPIO_ADDRESS + i));
        uart_puts("\n");
        
        if (filesize) 
            i += ((filesize + 3) & (-4));
    }
}

void cpio_cat(const char *filename) {
    int i;
    int filesize = 0;
    int namesize = 0;
    struct cpio_newc_header *header;

    for (i=0 ; ; i+=namesize) {
        header = ((struct cpio_newc_header *)(CPIO_ADDRESS + i));
        if (strncmp(header->c_magic, CPIO_MAGIC, 6)) {
            uart_puts("cpio: Bad magic\n");
            break;
        }
        filesize = atoi(header->c_filesize, 16, 8);
        namesize = atoi(header->c_namesize, 16, 8);

        i += sizeof(struct cpio_newc_header);
        if (!strncmp((char *)(CPIO_ADDRESS + i), filename, 10)) {
            uart_puts((char *)(CPIO_ADDRESS + i + namesize));
            uart_puts("\n");
            break;
        }
        if (!strncmp((char *)(CPIO_ADDRESS + i), CPIO_END, 10)) {
            uart_puts("File not exists...\n");
            break;
        }
        
        if (filesize) 
            i += ((filesize + 3) & (-4));
    }
}