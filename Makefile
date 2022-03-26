# OSC 2022
CC 		= aarch64-linux-gnu-gcc
LD 		= aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy


IMG         = kernel8.img
ELF         = kernel8.elf
LINKER_FILE = kern/linker.ld

OUT_DIR = out
INC_DIR = include

SRCS   := $(wildcard kern/*.S)
SRCS   += $(wildcard kern/*.c)
SRCS   += $(wildcard lib/*.c)
SRCS   := $(notdir $(SRCS))
OBJS   := $(patsubst %.c, $(OUT_DIR)/%.o, $(SRCS))
OBJS   := $(patsubst %.S, $(OUT_DIR)/%.o, $(OBJS))

# -fno-stack-protector: to disable stack protection
# -fno-builtin is required to avoid refs to undefined functions in the kernel.
# Only optimize to -O1 to discourage inlining, which complicates backtraces.
CFLAGS := -O1 -fno-builtin -nostdinc
CFLAGS += -Wall -I$(INC_DIR) -c -fno-stack-protector

.PHONY: asm debug clean run


$(IMG): $(ELF)
	$(OBJCOPY) -O binary $(ELF) $(IMG)
$(ELF): $(OBJS) $(LINKER_FILE $(OUT_DIR)/boot.o
	$(LD) -T $(LINKER_FILE) -o $(ELF) $(OBJS)

$(OUT_DIR)/%.o: kern/%.S $(OUT_DIR)
	$(CC) $(CFLAGS) $< -o $@
$(OUT_DIR)/%.o: kern/%.c $(OUT_DIR)
	$(CC) $(CFLAGS) $< -o $@
$(OUT_DIR)/%.o: lib/%.c $(OUT_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OUT_DIR):  
	@mkdir -p $(OUT_DIR)


asm: $(IMG)
	qemu-system-aarch64 -M raspi3 -kernel $(IMG) -display none -d in_asm
debug: $(IMG)
	qemu-system-aarch64 -M raspi3 -kernel $(IMG) -display none -serial null -serial stdio -initrd initramfs.cpio -S -s
run: $(IMG)
	qemu-system-aarch64 -M raspi3 -kernel $(IMG) -display none -serial null -serial stdio -initrd initramfs.cpio
clean:
	rm -rf $(OUT_DIR) $(ELF) $(IMG)