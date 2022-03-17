#include "peripheral/uart.h"
#include "peripheral/mailbox.h"
#include "kern/shell.h"
#include "simple_alloc.h"
#include "dtb.h"
#include "cpio.h"

#define MAX_BUFFER_SIZE 200

int main() {
    char *cmd;
    
    uart_init();
    uart_read();

    uart_puts("##########################################\n");
    get_board_revision();
    get_ARM_memory();
    uart_puts("##########################################\n");

    fdt_init();
    fdt_traverse(initramfs_callback);

    cmd = simple_malloc(MAX_BUFFER_SIZE);
    while (1) {
        uart_puts("raspi3> ");
        shell_input(cmd);
        shell_parse(cmd);
    }
    return 0;
}