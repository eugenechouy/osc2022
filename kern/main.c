#include "peripheral/mailbox.h"
#include "kern/shell.h"
#include "kern/timer.h"
#include "kern/irq.h"
#include "kern/sched.h"
#include "kern/kio.h"
#include "kern/cpio.h"
#include "kern/mm.h"
#include "dtb.h"

void hw_info() {
    unsigned int result[2];
    kputs("##########################################\n");
    
    get_board_revision(result);
    kputs("Board revision:\t\t\t0x");
    kputn(result[0], 16);
    kputs("\n");

    get_ARM_memory(result);
    kputs("ARM memory base address:\t0x");
    kputn(result[0], 16);
    kputs("\n");
    kputs("ARM memory size:\t\t0x");
    kputn(result[1], 16);
    kputs("\n");
    
    kputs("##########################################\n");
}

void rootfs_init() {
    if (fdt_init() < 0) {
        kputs("dtb: Bad magic\n");
        return;
    }
    if (fdt_traverse(initramfs_callback) < 0)
        kputs("dtb: Unknown token\n");
    kputs("dtb: init success\n");
}

void kmalloc_test() {
    void *addr = kmalloc(19);
    kputn((long)addr, 16);
    kputs("\n");

    kfree(addr);

    void *addr2 = kmalloc(20);
    kputn((long)addr2, 16);
    kputs("\n");

    addr = kmalloc(19);
    kputn((long)addr, 16);
    kputs("\n");

    addr = kmalloc(256);
    kputn((long)addr, 16);
    kputs("\n");

    addr = kmalloc(8142);
    kputn((long)addr, 16);
    kputs("\n");
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

    kmalloc_test();

    shell_start();
}