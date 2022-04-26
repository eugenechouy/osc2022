#ifndef SYSCALL_H
#define SYSCALL_H

#define SYS_GET_PID     1
#define SYS_UART_READ   2
#define SYS_UART_WRITE  3
#define SYS_EXEC        4
#define SYS_FORK        5
#define SYS_EXIT        6
#define SYS_MBOX_CALL   7
#define SYS_KILL        8

int getpid();
int uart_read(char buf[], int size);
int uart_write(const char buf[], int size);
int exec(const char *name, char *const argv[]);
int fork();
void exit();
// extern int mbox_call(unsigned char ch, unsigned int *mbox)
// extern void kill(int pid);

#endif