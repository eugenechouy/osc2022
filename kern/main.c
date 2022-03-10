#include "peripheral/uart.h"
#include "peripheral/mailbox.h"
#include "kern/shell.h"

#define MAX_BUFFER_SIZE 200

int main() {
    char cmd[MAX_BUFFER_SIZE];
    
    uart_puts("\n");
    uart_init();

    get_board_revision();
    get_ARM_memory();

    while (1) {
        uart_puts("rp3> ");
        shell_input(cmd);
        shell_parse(cmd);
        // uart_write(uart_read());
    }
    return 0;
}