#include "peripheral/mailbox.h"
#include "kern/shell.h"
#include "kern/timer.h"
#include "kern/irq.h"
#include "kern/sched.h"
#include "kern/kio.h"
#include "kern/cpio.h"
#include "kern/mm.h"
#include "dtb.h"
#include "startup_alloc.h"
#include "syscall.h"
#include "string.h"

void hw_info() {
    unsigned int result[2];
    kputs("##########################################\n");
    get_board_revision(result);
    kprintf("Board revision:\t\t\t0x%x\n", result[0]);
    get_ARM_memory(result);
    kprintf("ARM memory base address:\t0x%x\n", result[0]);
    kprintf("ARM memory size:\t\t0x%x\n", result[1]);
    kputs("##########################################\n");
}

void rootfs_init() {
    if (fdt_init() < 0) {
        kputs("dtb: Bad magic\n");
        return;
    }
    if (fdt_traverse(initramfs_callback) < 0)
        kputs("dtb: Unknown token\n");
    if (fdt_traverse(mm_callback) < 0)
        kputs("dtb: Unknown token\n");
    kputs("dtb: init success\n");
}

extern unsigned int __stack_kernel_top;

void reserve_memory() {
    // page used by startup allocator
    reserved_kern_startup();
    // device tree 
    fdt_reserve();
    // initramfs
    cpio_reserve();
    // initial kernel stack: 2 pages
    mm_reserve((void *)&__stack_kernel_top - 0x2000, (void *)&__stack_kernel_top);
}

void delay(int times) {
    while(times--) {
        asm volatile("nop");
    }
}

void foo(){
    for(int i = 0; i < 10; ++i) {
        kprintf("Thread id: %d %d\n", get_current()->tid, i);
        delay(1e8);
        schedule();
    }
    exit();
}

#include "user_lib.h"

void test_mbox(){
    unsigned int mailbox[7];
    mailbox[0] = 7 * 4;
    mailbox[1] = REQUEST_CODE;
    mailbox[2] = GET_BOARD_REVISION; 
    mailbox[3] = 4;
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0; 
    mailbox[6] = END_TAG;
    if (mbox_call(0x8, mailbox)) {
        // it should be 0xa020d3 for rpi3 b+
        printf("revision %x\n", mailbox[5]);
    }
}

void fork_test() {
    printf("\nFork Test, pid %d\n", getpid());
    int cnt = 1;
    int ret = 0;
    if ((ret = fork()) == 0) { // child
        long long cur_sp;
        asm volatile("mov %0, sp" : "=r"(cur_sp));
        printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
        ++cnt;

        if ((ret = fork()) != 0){
            asm volatile("mov %0, sp" : "=r"(cur_sp));
            printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
        }
        else{
            while (cnt < 5) {
                asm volatile("mov %0, sp" : "=r"(cur_sp));
                printf("second child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
                delay(1000000);
                ++cnt;
            }
        }
        exit();
    }
    else {
        printf("parent here, pid %d, child %d\n", getpid(), ret);
    }
    exit(); 
}

void sig_handler() {
    kprintf("my handler\n");
    return;
}

void sig_test() {
    signal(1, sig_handler);
    sigkill(2, 1);
    exit();
}

char *user_prog1;

void user_prog() {
    // do_exec(sig_test);
    exec("syscall.img", "");
}

void idle_task() {
    while(1) {
        schedule();
    }
}

void kern_main() { 
    kio_init();
    runqueue_init();
    task_init();
    int_init();
    core_timer_enable();
    unsigned long tmp;
    asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
    tmp |= 1;
    asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));
    timer_sched_latency();

    kputs("press any key to continue...");
    // kscanc();
    rootfs_init();
    hw_info();

    mm_init();
    reserve_memory();

    // cpio_exec("syscall.img");
    // for(int i = 0; i < 10; ++i) { // N should > 2
    //     thread_create(foo);
    // }
    // privilege_task_create(shell_start, 10);
    // thread_create(fork_test);
    privilege_task_create(user_prog, 10);
    // privilege_task_create(kill_zombies, 10);
    idle_task();
}