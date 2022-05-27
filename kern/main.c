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

void dtb_init() {
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
    kprintf("page used by startup allocator\n");
    reserved_kern_startup();
    kprintf("device tree\n");
    fdt_reserve();
    kprintf("initramfs\n");
    cpio_reserve();
    kprintf("initial kernel stack\n");
    mm_reserve((void *)PHY_2_VIRT((void *)&__stack_kernel_top - 0x2000), (void *)PHY_2_VIRT((void *)&__stack_kernel_top));
}

void delay(int times) {
    while(times--) {
        asm volatile("nop");
    }
}

#include "user_lib.h"

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

char *user_code;

void user_prog() {
    user_code = cpio_find("vm.img");
    if (user_code)
        __exec(user_code, 0);
    exit();
}

void idle_task() {
    while(1) {
        schedule();
    }
}

void test_fs() {
    char buf[8];
    mkdir("mnt", 0);
    int fd = open("/mnt/a.txt", O_CREAT);
    write(fd, "Hi", 2);
    close(fd);
    chdir("mnt");
    fd = open("./a.txt", 0);
    if (fd < 0) {
        kprintf("error1\n");
        return;
    }
    read(fd, buf, 2);
    if (strncmp(buf, "Hi", 2) != 0) {
        kprintf("error2\n");
        return;
    }

    chdir("..");
    mount("", "mnt", "tmpfs", 0, 0);
    fd = open("mnt/a.txt", 0);
    if (fd > 0) {
        kprintf("error3\n");
        return;
    }
}

void kern_main() { 
    kio_init();
    mm_init();
    rootfs_init();
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
    kscanc();
    kputs("\n");
    dtb_init();
    hw_info();

    reserve_memory();

    test_fs();
    thread_create(user_prog);
    // privilege_task_create(kill_zombies, 10);
    idle_task();
}