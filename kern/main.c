#include "peripheral/uart.h"
#include "peripheral/mailbox.h"
#include "kern/shell.h"
#include "kern/timer.h"
#include "kern/irq.h"
#include "kern/sched.h"
#include "simple_alloc.h"
#include "dtb.h"
#include "cpio.h"

int main() {
    char *cmd;
    
    uart_init();
    timer_init();
    uart_flush();

    init_task_queue();
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