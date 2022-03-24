#include "peripheral/uart.h"
#include "string.h"
#include "dtb.h"
#include "byteswap.h"

#define align4(num) ((num + 3) & (-4))

char FDT_HEADER_MAGIC[4] = {0xd0, 0x0d, 0xfe, 0xed};

void *DTB_ADDRESS;

int fdt_init() {
    asm volatile("MOV %0, x23" :  "=r"(DTB_ADDRESS));
    if (strncmp((char*)DTB_ADDRESS, FDT_HEADER_MAGIC, 4))
        return -1;
    return 0;
}

void fdt_traverse(void (*cb)(char *, char *, void *)) {
    unsigned int token;
    void *ptr;
    void *struct_block;
    void *string_block;
    char *node_name = 0;
    char *prop_name = 0;
    struct fdt_prop *prop;
    struct fdt_header *header = DTB_ADDRESS;

    struct_block = DTB_ADDRESS + __bswap_32(header->off_dt_struct);
    string_block = DTB_ADDRESS + __bswap_32(header->off_dt_strings);

    for (ptr=struct_block ; ; ptr+=4) {
        token = __bswap_32(*((unsigned int*)ptr));
        if (token == FDT_BEGIN_NODE) {
            node_name = ptr + 4;
            ptr += align4(strlen(node_name) + 1);
        } else if (token == FDT_END_NODE) {
        } else if (token == FDT_PROP) {
            prop = ptr + 4;
            prop_name = string_block + __bswap_32(prop->nameoff);
            cb(node_name, prop_name, ptr + 12);
            ptr += align4(8 + __bswap_32(prop->len));
        } else if (token == FDT_NOP) {
        } else if (token == FDT_END) {
            break;
        } else {
            uart_puts("dtb: unknown TOKEN\n");
            break;
        }
    }
}