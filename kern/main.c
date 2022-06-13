#include "peripheral/mailbox.h"
#include "kern/shell.h"
#include "kern/timer.h"
#include "kern/irq.h"
#include "kern/sched.h"
#include "kern/kio.h"
#include "kern/cpio.h"
#include "kern/mm.h"
#include "kern/pagecache.h"
#include "dtb.h"
#include "startup_alloc.h"
#include "syscall.h"
#include "string.h"
#include "fs/vfs.h"
#include "fs/fat32.h"
#include "drivers/sdhost.h"
#include "drivers/mbr.h"

void sd_mount() {
    struct fat32_boot_sector fat32_bs;
    struct fat32_fsinfo_sector fat32_fsinfo;
    struct mbr mbr;

    sd_init();
    readblock(0, &mbr);

    if (mbr.mbr_signature != MBR_SIGNATURE) {
        kprintf("sdcard mbr signature error: %x\n", mbr.mbr_signature);
        return;
    }

    if (mbr.partition[0].partition_type == 0xB) {
        readblock(mbr.partition[0].first_sector_lba, &fat32_bs);

        fat32_info.sec_size  = fat32_bs.BPB_BytsPerSec;
        fat32_info.clus_size = fat32_bs.BPB_SecPerClus;
        fat32_info.first_lba = mbr.partition[0].first_sector_lba;
        fat32_info.fat_lba   = mbr.partition[0].first_sector_lba + fat32_bs.BPB_RsvdSecCnt;
        fat32_info.fat_num   = fat32_bs.BPB_NumFATs;
        fat32_info.fat_size  = fat32_bs.BPB_FATSz32;
        fat32_info.data_lba  = mbr.partition[0].first_sector_lba + fat32_bs.BPB_RsvdSecCnt + (fat32_bs.BPB_NumFATs * fat32_bs.BPB_FATSz32);
        fat32_info.root_clus = fat32_bs.BPB_RootClus;
        fat32_info.total_sec = fat32_bs.BPB_TotSec32;
        
        readblock(mbr.partition[0].first_sector_lba + fat32_bs.BPB_FSInfo, &fat32_fsinfo);
        if (fat32_fsinfo.FSI_LeadSig == FSI_LEAD_SIG && fat32_fsinfo.FSI_StrucSig == FSI_STRUCT_SIG && fat32_fsinfo.FSI_Nxt_Free != FSI_NXT_FREE_UNKNOWN) {
            fat32_info.fsi_next_free = fat32_fsinfo.FSI_Nxt_Free;
        } else {
            fat32_info.fsi_next_free = 2;
        }
       
        vfs_mkdir("/boot");
        vfs_mount("/boot", "fat32");
    }
}

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

void user_prog() {
    exec("/initramfs/vfs2.img", 0);
    exit();
}

void idle_task() {
    while(1) {
        schedule();
    }
}

void initramfs_init() {
    vfs_mkdir("/initramfs");
    vfs_mount("/initramfs", "initramfs");
}

#include "test_func.h"

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

    initramfs_init();
    pagecache_init();
    sd_mount();
    // fs_test();
    fat_test();
    
    thread_create(user_prog);
    privilege_task_create(kill_zombies, 10);
    idle_task();
}