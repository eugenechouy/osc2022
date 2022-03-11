#include "peripheral/uart.h"
#include "string.h"

#define KERNEL_ADDR 0x80000
#define READ_INT(var) \
            var = 0; \
            for (i=0 ; i<4 ; i++) {\
                var <<= 8; \
                var |= (int)uart_read_raw(); \
            } \

int main() {
    int img_size;
    int img_checksum;
    int i;
    char buffer[64];

    uart_init();
    uart_puts("You can start to send kernel image via UART...\n");

    READ_INT(img_size);
    READ_INT(img_checksum);

    itoa(img_size, buffer, 10);
    uart_puts(buffer); 
    uart_puts("\n");
    itoa(img_checksum, buffer, 10);
    uart_puts(buffer); 
    uart_puts("\n");

    for (i=0 ; i<img_size ; i++) {
        char b = uart_read_raw();
        *((char*)KERNEL_ADDR + i) = b;
        img_checksum -= (int)b;
    }

    if (img_checksum != 0) {
        uart_puts("Send img failed...\n");
    } else {
        uart_puts("Send img success!\n");
        void (*start)(void) = (void *)KERNEL_ADDR;
        start();
    }

    return 0;
}