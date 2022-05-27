#include "kern/kio.h"
#include "kern/shell.h"
#include "kern/timer.h"
#include "kern/sched.h"
#include "kern/cpio.h"
#include "kern/mm.h"
#include "string.h"
#include "reset.h"

void shell_input(char *cmd) {
    char c;
    unsigned int len = 0;

    while((c = kscanc()) != '\n') {
        if (c == BACKSPACE || c == DELETE) {
            if (!len) 
                continue;
            LEFT_SHIFT
            kputc(' ');
            LEFT_SHIFT
            --len;
        } else if (c == ESC) {
            kscanc();
            kscanc();
        } else { // regular letter
            kputc(c);
            cmd[len++] = c;
        } 
    }
    kputs("\n");
    cmd[len] = '\0';
}

void shell_help() {
    kputs("help\t\t: print this help menu\n");
    kputs("hello\t\t: print Hello World!\n");
    kputs("ls\t\t: list file\n");
    kputs("cat\t\t: print file content\n");
    kputs("exec\t\t: execute a file\n");
    kputs("setTimeout\t: MESSAGE SECONDS\n");
    kputs("reboot\t\t: reboot the device\n");
}

void shell_parse(char *cmd) {
    char args[MAX_INPUT_LEN];
    if (!strcmp(cmd, "help")) {
        shell_help();              
    } else if (!strcmp(cmd, "hello")) {
        kputs("Hello World!\n");
    } else if (!strcmp(cmd, "ls")) {
        cpio_ls();
    } else if (!strcmp(cmd, "cat")) {
        kputs("FileName: ");
        shell_input(args);
        cpio_cat(args);
    } else if (!strcmp(cmd, "setTimeout")) {
        shell_input(args);
        set_timeout(args);
    } else if (!strcmp(cmd, "reboot")) {
        kputs("About to reboot...\n");
        reset(1000);
    } else {
        kputs(cmd);
        kputs(": command not found\n");
    }
}

void shell_start() {
    char cmd[128];
    while (1) {
        kputs("raspi3> ");
        shell_input(cmd);
        shell_parse(cmd);
    }
}