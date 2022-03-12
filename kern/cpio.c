#include "kern/cpio.h"
#include "peripheral/uart.h"
#include "string.h"

void cpio_parse() {
    int i;
    int filesize = 0;
    int namesize = 0;
    struct cpio_newc_header *header;

    for (i=0 ; ; i+=namesize) {
        header = ((struct cpio_newc_header *)(CPIO_ADDRESS + i));
        if (strncmp(header->c_magic, CPIO_MAGIC, 6)) {
            uart_puts("cpio: Bad magic");
            break;
        }
        filesize = atoi(header->c_filesize, 16, 8);
        namesize = atoi(header->c_namesize, 16, 8);

        i += sizeof(struct cpio_newc_header);
        if (!strncmp((char *)(CPIO_ADDRESS + i), CPIO_END, 10))
            break;
        uart_puts((char *)(CPIO_ADDRESS + i));
        uart_puts("\n");
        
        if (filesize)  {
            i += filesize + (4 - filesize % 4);
        } 
    }
}