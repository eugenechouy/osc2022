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
#define SYS_SIGNAL      8
#define SYS_SIGKILL     9

int getpid();
int uart_read(char buf[], int size);
int uart_write(const char buf[], int size);
int exec(const char *name, char *const argv[]);
int fork();
void exit();
int mbox_call(unsigned char ch, unsigned int *mbox);
void kill(int pid);
void signal(int SIGNAL, void (*handler)());
void sigkill(int pid, int SIGNAL);

#define SYS_OPEN        11
#define SYS_CLOSE       12
#define SYS_WRITE       13
#define SYS_READ        14
#define SYS_MKDIR       15
#define SYS_MOUNT       16
#define SYS_CHDIR       17
#define SYS_LSEEK64     18

#define O_CREAT 00000100

int open(const char *pathname, int flags);
int close(int fd);
long write(int fd, const void *buf, unsigned long count);
long read(int fd, void *buf, unsigned long count);
int mkdir(const char *pathname, unsigned mode);
int mount(const char *src, const char *target, const char *filesystem, unsigned long flags, const void *data);
int chdir(const char *path);

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

long lseek64(int fd, long offset, int whence);

#endif