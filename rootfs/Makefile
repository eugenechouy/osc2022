# OSC 2022
CC 		= aarch64-linux-gnu-gcc
LD 		= aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy

LINKER_FILE = linker.ld

# -fno-stack-protector: to disable stack protection
# -fno-builtin is required to avoid refs to undefined functions in the kernel.
# Only optimize to -O1 to discourage inlining, which complicates backtraces.
CFLAGS := -O1 -fno-builtin -nostdinc
CFLAGS += -Wall -c -fno-stack-protector

user_prog: user_prog.S $(LINKER_FILE)
	$(CC) $(CFLAGS) $< -o $@.o
	$(LD) -T $(LINKER_FILE) -o $@ $@.o
	$(OBJCOPY) $@ -O binary $@

archv: 
	find . | cpio -o -H newc > ../initramfs.cpio