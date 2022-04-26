#ifndef SYSCALL_H
#define SYSCALL_H

#define SYS_GET_PID     0
#define SYS_UART_READ   1
#define SYS_UART_WRITE  2
#define SYS_EXEC        3
#define SYS_FORK        4
#define SYS_EXIT        5
#define SYS_MBOX_CALL   6
#define SYS_KILL        7

int getpid();
int uart_read(char buf[], int size);
int uart_write(const char buf[], int size);
int exec(const char *name, char *const argv[]);
int fork();
void exit();
int mbox_call(unsigned char ch, unsigned int *mbox);
void kill(int pid);

#endif