#include "peripheral/uart.h"
#include "peripheral/mailbox.h"
#include "kern/shell.h"
#include "simple_alloc.h"
#include "dtb.h"
#include "cpio.h"

void int_enable() {
    asm volatile("msr DAIFClr, 0xf");
}

void int_disable() {
    asm volatile("msr DAIFSet, 0xf");
}

int main() {
    char *cmd;
    
    uart_init();
    uart_flush();

    int_enable();
    uart_enable_int();
    
    uart_read();
    uart_puts("##########################################\n");
    get_board_revision();
    get_ARM_memory();
    uart_puts("##########################################\n");

    if (fdt_init() >= 0) {
        uart_puts("dtb: correct magic\n");
        fdt_traverse(initramfs_callback);
    } else 
        uart_puts("dtb: Bad magic\n");

    cmd = simple_malloc(128);
    while (1) {
        uart_puts("raspi3> ");
        shell_input(cmd);
        shell_parse(cmd);
    }
    return 0;
}