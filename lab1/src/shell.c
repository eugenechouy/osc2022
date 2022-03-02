#include "uart.h"
#include "string.h"
#include "shell.h"
#include "reset.h"

void shell_input(char *cmd) {
    char c;
    unsigned int len = 0;

    while((c = uart_read()) != '\n') {
        if (c == BACKSPACE || c == DELETE) {
            LEFT_SHIFT
            uart_write(' ');
            LEFT_SHIFT
            --len;
        } else if (c == ESC) {
            uart_read();
            uart_read();
        } else { // regular letter
            uart_write(c);
            cmd[len++] = c;
        } 
    }
    uart_puts("\n");
    cmd[len] = '\0';
}

void shell_parse(char *cmd) {
    if (!strcmp(cmd, "help")) {                      
        uart_puts("help\t: print this help menu\n");
        uart_puts("hello\t: print Hello World!\n");
        uart_puts("reboot\t: reboot the device\n");
    } else if (!strcmp(cmd, "hello")) {
        uart_puts("Hello World!\n");
    } else if (!strcmp(cmd, "reboot")) {
        uart_puts("About to reboot...\n");
        reset(1000);
    } else {
        uart_puts(cmd);
        uart_puts(": command not found\n");
    }
}