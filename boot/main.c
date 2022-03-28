#include "peripheral/uart.h"

#define KERNEL_ADDR 0x80000
#define READ_INT(var) \
            var = 0; \
            for (i=0 ; i<4 ; i++) {\
                var <<= 8; \
                var |= (int)uart_sync_read_raw(); \
            } \

int main() {
    int img_size;
    int img_checksum;
    int i;

    uart_init();
    uart_flush();

    READ_INT(img_size);
    READ_INT(img_checksum);

    uart_sync_printNum(img_size, 10);
    uart_sync_puts("\n");
    uart_sync_printNum(img_checksum, 10);
    uart_sync_puts("\n");

    for (i=0 ; i<img_size ; i++) {
        char b = uart_sync_read_raw();
        *((char*)KERNEL_ADDR + i) = b;
        img_checksum -= (int)b;
    }

    if (img_checksum != 0) {
        uart_sync_puts("Send img failed...\n");
    } else {
        uart_sync_puts("Send img success!\n");
        void (*start)(void) = (void *)KERNEL_ADDR;
        start();
    }

    return 0;
}