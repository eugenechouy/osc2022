#include "uart.h"
#include "shell.h"

#define MAX_BUFFER_SIZE 200

int main() {
    uart_init();

    uart_puts("UART initialized successfully! Type anything to try ...\n");

    while (1) {
        uart_write(uart_read());
    }
    return 0;
}