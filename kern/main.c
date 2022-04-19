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
    // kernel stack 1MB
    mm_reserve((void *)&__stack_kernel_top - 0x100000, (void *)&__stack_kernel_top);
}

void kern_main() { 
    kio_init();
    timer_init();
    task_queue_init();
    int_enable();

    kputs("press any key to continue...");
    kscanc();
    rootfs_init();
    hw_info();

    mm_init();
    reserve_memory();

    shell_start();
}