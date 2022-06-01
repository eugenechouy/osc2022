#include "kern/timer.h"
#include "kern/kio.h"
#include "kern/cpio.h"
#include "kern/sched.h"
#include "kern/irq.h"
#include "kern/signal.h"
#include "kern/sync.h"
#include "kern/page.h"
#include "reset.h"
#include "syscall.h"
#include "peripheral/mailbox.h"
#include "fs/vfs.h"


inline void sys_getpid(struct trapframe *trapframe) {
    trapframe->x[0] = __getpid();
}

inline void sys_uart_read(struct trapframe *trapframe) {
    int i;
    int size = trapframe->x[1];
    char *buf = (char *)trapframe->x[0];
    for(i=0 ; i<size ; i++)
        buf[i] = uart_async_read();
    trapframe->x[0] = i;
}

inline void sys_uart_write(struct trapframe *trapframe) {
    int i;
    char *buf = (char *)trapframe->x[0];
    int size = trapframe->x[1];
    for(i=0 ; i<size ; i++) {
        if (buf[i] == '\n')
            uart_async_write('\r');
        uart_async_write(buf[i]);
    }
    trapframe->x[0] = i;
}

inline void sys_exec(struct trapframe *trapframe) {
    const char *name = (const char *)trapframe->x[0];
    __exec(name, (void*)trapframe->x[1]);
    trapframe->x[0] = 0;   
}

inline void sys_fork(struct trapframe *trapframe) {
    trapframe->x[0] = __fork(trapframe);
}

inline void sys_exit(struct trapframe *trapframe) {
    __exit();
}

inline void sys_mbox_call(struct trapframe *trapframe) {
    unsigned char ch = trapframe->x[0];
    unsigned int *mailbox = (unsigned int *)trapframe->x[1];
    unsigned int *kernel_addr = walk((struct mm_struct *)&(get_current()->mm.pgd), (unsigned long)mailbox, 0);
    trapframe->x[0] = mailbox_call(ch, kernel_addr);
}

inline void sys_kill(struct trapframe *trapframe) {
    __kill(trapframe->x[0]);
}

inline void sys_signal(struct trapframe *trapframe) {
    __signal(trapframe->x[0], (void*)trapframe->x[1]);
}

inline void sys_sigkill(struct trapframe *trapframe) {
    __sigkill(trapframe->x[0], trapframe->x[1], trapframe);
}

inline void sys_sigreturn(struct trapframe *trapframe) {
    signal_back(trapframe);
}

inline void sys_open(struct trapframe *trapframe) {
    int fd;
    struct file *fh;
    const char *pathname = (char *)trapframe->x[0];
    int flags = trapframe->x[1];
    int ret = vfs_open(pathname, flags, &fh);
#ifdef DEBUG_FS
    kprintf("open(%s, %d)\n", pathname, flags);
#endif
    if (ret < 0)  {
        trapframe->x[0] = ret;
        return;
    }
    fd = fd_open(&(get_current()->files), fh);
    trapframe->x[0] = fd;
}

inline void sys_close(struct trapframe *trapframe) {
    struct file *fh;
    int fd = trapframe->x[0];
#ifdef DEBUG_FS
    kprintf("close(%d)\n", fd);
#endif
    if (fd < 0) {
        trapframe->x[0] = -1;
        return;
    }
    fh = fd_close(&(get_current()->files), fd);
    trapframe->x[0] = vfs_close(fh);
}

inline void sys_write(struct trapframe *trapframe) {
    struct file *fh;
    int fd = trapframe->x[0];
    const char *buf = (char *)trapframe->x[1];
    unsigned long count = trapframe->x[2];
#ifdef DEBUG_FS
    kprintf("write(%d, %s, %d)\n", fd, buf, count);
#endif
    if (fd < 0) {
        trapframe->x[0] = -1;
        return;
    }
    fh = fd_get(&(get_current()->files), fd);
    if (fh == 0) {
        trapframe->x[0] = -1;
        return;
    }
    trapframe->x[0] = vfs_write(fh, buf, count);
}

inline void sys_read(struct trapframe *trapframe) {
    struct file *fh;
    int fd = trapframe->x[0];
    char *buf = (char *)trapframe->x[1];
    unsigned long count = trapframe->x[2];
#ifdef DEBUG_FS 
    kprintf("read(%d, %d)\n", fd, count);
#endif
    if (fd < 0) {
        trapframe->x[0] = -1;
        return;
    }
    fh = fd_get(&(get_current()->files), fd);
    if (fh == 0) {
        trapframe->x[0] = -1;
        return;
    }
    trapframe->x[0] = vfs_read(fh, buf, count);
}

inline void sys_mkdir(struct trapframe *trapframe) {
    const char *pathname = (char *)trapframe->x[0];
#ifdef DEBUG_FS
    kprintf("mkdir(%s)\n", pathname);
#endif
    // unsigned mode = trapframe->x[1];
    trapframe->x[0] = vfs_mkdir(pathname);
}

inline void sys_mount(struct trapframe *trapframe) {
    const char *target = (char *)trapframe->x[1];
    const char *filesystem = (char *)trapframe->x[2];
#ifdef DEBUG_FS
    kprintf("mount(%s, %s)\n", target, filesystem);
#endif
    trapframe->x[0] = vfs_mount(target, filesystem);
}

inline void sys_chdir(struct trapframe *trapframe) {
    const char *path = (char *)trapframe->x[0];
#ifdef DEBUG_FS
    kprintf("chdir(%s)\n", path);
#endif
    trapframe->x[0] = vfs_chdir(path);
}

inline void sys_lseek64(struct trapframe *trapframe) {
    struct file *fh;
    int fd      = trapframe->x[0];
    long offset = trapframe->x[1];
    int whence  = trapframe->x[2];

    if (fd < 0) {
        trapframe->x[0] = -1;
        return;
    }
    fh = fd_get(&(get_current()->files), fd);
    if (fh == 0) {
        trapframe->x[0] = -1;
        return;
    }
    trapframe->x[0] = vfs_lseek64(fh, offset, whence);
}

void syscall_main(struct trapframe *trapframe) {
    int_enable();
    long syscall_num = trapframe->x[8];
    switch(syscall_num) {
        case SYS_GET_PID:
            sys_getpid(trapframe);
            break;
        case SYS_UART_READ:
            sys_uart_read(trapframe);
            break;
        case SYS_UART_WRITE:
            sys_uart_write(trapframe);
            break;
        case SYS_EXEC:
            sys_exec(trapframe);
            break;
        case SYS_FORK:
            sys_fork(trapframe);
            break;
        case SYS_EXIT:
            sys_exit(trapframe);
            break;
        case SYS_MBOX_CALL:
            sys_mbox_call(trapframe);
            break;
        case SYS_KILL:
            sys_kill(trapframe);
            break;
        case SYS_SIGNAL:
            sys_signal(trapframe);
            break;
        case SYS_SIGKILL:
            sys_sigkill(trapframe);
            break;
        case SYS_OPEN:
            sys_open(trapframe);
            break;
        case SYS_CLOSE:
            sys_close(trapframe);
            break;
        case SYS_WRITE:
            sys_write(trapframe);
            break;
        case SYS_READ:
            sys_read(trapframe);
            break;
        case SYS_MKDIR:
            sys_mkdir(trapframe);
            break;
        case SYS_MOUNT:
            sys_mount(trapframe);
            break;
        case SYS_CHDIR:
            sys_chdir(trapframe);
            break;
        case SYS_LSEEK64:
            sys_lseek64(trapframe);
            break;
        case 30:
            sys_sigreturn(trapframe);
            break;
        default:
            uart_sync_puts("Undefined syscall number, about to reboot...\n");
            reset(1000);
            while(1);
    }
    int_disable();
}

void svc_main(unsigned long spsr, unsigned long elr, unsigned long esr, struct trapframe *trapframe) {
    unsigned int svc_num;
    svc_num = esr & 0xFFFFFF;    

    switch(svc_num) {
    case 0:
        syscall_main(trapframe);
        break;
    case 1:
        kputs("svc 1\n");
        core_timer_enable();
        break;
    case 2:
        /*
        bits [31:26] 0b010101 SVC instruction execution in AArch64 state.
        */
        kprintf("\nspsr_el1: \t%x\n", spsr);
        kprintf("elr_el1: \t%x\n", elr);
        kprintf("esr_el1: \t%x\n", esr);
        break;
    default:
        uart_sync_printNum(svc_num, 10);
        uart_sync_puts(": Undefined svc number, about to reboot...\n");
        reset(1000);
        while(1);
    }
}

void fault_parse(unsigned long sc) {
    unsigned long far;
    if (sc & 0b110000) {
        kprintf("Others fault...\n");
        return;
    }

    switch((sc & 0b1100) >> 2) {
    case 0:
        kprintf("Address size fault,");
        break;
    case 1:
        kprintf("Translation fault,");
        break;
    case 2:
        kprintf("Access flag fault,");
        break;
    case 3:
        kprintf("Permission fault,");
        break;
    }
    
    kprintf(" level %d\n", sc & 0b11);
    asm volatile("mrs %0, far_el1":"=r"(far));
    kprintf("Address: \t0x%x\n", far);
}

void sync_main(unsigned long spsr, unsigned long elr, unsigned long esr, struct trapframe *trapframe) {
    switch(EC_BITS(esr)) {
    case EC_SVC_64:
        svc_main(spsr, elr, esr, trapframe);
        break;
    case EC_IA_EL0:
        kprintf("(EL0)");
    case EC_IA_EL1:
        kprintf("Instruction Abort\n");
        fault_parse(IFSC(esr));
        kprintf("spsr_el0: \t0x%x\n", trapframe->sp_el0);
        __exit();
        break;
    case EC_DA_EL0:
        kprintf("(EL0)");
    case EC_DA_EL1:
        kprintf("Data Abort\n");
        fault_parse(DFSC(esr));
        kprintf("spsr_el0: \t0x%x\n", trapframe->sp_el0);
        __exit();
        break;
    default:
        uart_sync_printNum(EC_BITS(esr), 10);
        uart_sync_puts(": Unknown ec bit, about to reboot...\n");
        reset(1000);
        while(1);
    }
}