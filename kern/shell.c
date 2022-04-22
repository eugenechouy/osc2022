#include "kern/kio.h"
#include "kern/shell.h"
#include "kern/timer.h"
#include "kern/sched.h"
#include "kern/cpio.h"
#include "kern/mm.h"
#include "string.h"
#include "reset.h"

#define TIME 1e7

void dummpy_task2() {
    kputs("dummy task high start\n");
    int time = TIME;
    int cnt = 1;
    while(cnt--) {
        while(time--) asm volatile("nop");
        time = TIME;
    }
    kputs("dummy task high end\n");
}

void dummpy_task1() {
    kputs("dummy task low start\n");
    int time = TIME;
    int cnt = 5;
    while(time--) asm volatile("nop");
    task_create(dummpy_task2, 0, 5);
    while(cnt--) {
        while(time--) asm volatile("nop");
        time = TIME;
    }
    kputs("dummy task low end\n");
}

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
    } else if (!strcmp(cmd, "exec")) {
        kputs("FileName: ");
        shell_input(args);
        cpio_exec(args);
    } else if (!strcmp(cmd, "setTimeout")) {
        shell_input(args);
        set_timeout(args);
    } else if (!strcmp(cmd, "reboot")) {
        kputs("About to reboot...\n");
        reset(1000);
    } else if (!strcmp(cmd, "test")) {
        task_create(dummpy_task1, 0, 10);
        kputs("\n");
    } else {
        kputs(cmd);
        kputs(": command not found\n");
    }
}

void shell_start() {
    // char *cmd;
    // cmd = kmalloc(128);
    char cmd[128];
    while (1) {
        kputs("raspi3> ");
        shell_input(cmd);
        shell_parse(cmd);
    }
}