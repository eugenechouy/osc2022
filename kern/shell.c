#include "peripheral/uart.h"
#include "kern/shell.h"
#include "string.h"
#include "reset.h"
#include "cpio.h"

void shell_input(char *cmd) {
    char c;
    unsigned int len = 0;

    while((c = uart_read()) != '\n') {
        if (c == BACKSPACE || c == DELETE) {
            if (!len) 
                continue;
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
    char args[30];
    if (!strcmp(cmd, "help")) {                      
        uart_puts("help\t: print this help menu\n");
        uart_puts("hello\t: print Hello World!\n");
        uart_puts("reboot\t: reboot the device\n");
    } else if (!strcmp(cmd, "hello")) {
        uart_puts("Hello World!\n");
    } else if (!strcmp(cmd, "ls")) {
        cpio_ls();
    } else if (!strcmp(cmd, "reboot")) {
        uart_puts("About to reboot...\n");
        reset(1000);
    } else if (!strcmp(cmd, "cat")) {
        uart_puts("FileName: ");
        shell_input(args);
        cpio_cat(args);
    } else if (!strcmp(cmd, "exec")) {
        uart_puts("FileName: ");
        shell_input(args);
        cpio_exec(args);
    } else {
        uart_puts(cmd);
        uart_puts(": command not found\n");
    }
}