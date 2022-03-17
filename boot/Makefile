# bootloader
CC      = aarch64-linux-gnu-gcc
LD      = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy


IMG = bootloader.img
ELF = bootloader.elf

OUT_DIR = out
INC_DIR = ../include

SRCS   :=   boot.S \
			relocate.c \
            main.c \
            ../lib/uart.c \
			../lib/string.c
SRCS   := $(notdir $(SRCS))
OBJS   := $(patsubst %.c, $(OUT_DIR)/%.o, $(SRCS))
OBJS   := $(patsubst %.S, $(OUT_DIR)/%.o, $(OBJS))

# -fno-stack-protector: to disable stack protection
CFLAGS := -O1 -fno-builtin -nostdinc
CFLAGS += -Wall -I$(INC_DIR) -c -fno-stack-protector 

.PHONY: asm debug clean run pty


$(IMG): $(ELF)
	$(OBJCOPY) -O binary $(ELF) $(IMG)
$(ELF): $(OBJS) linker.ld $(OUT_DIR)/boot.o
	$(LD) -T linker.ld -o $(ELF) $(OBJS)

$(OUT_DIR)/boot.o: boot.S $(OUT_DIR)
	$(CC) $(CFLAGS) $< -o $@
$(OUT_DIR)/%.o: %.c $(OUT_DIR)
	$(CC) $(CFLAGS) $< -o $@
$(OUT_DIR)/%.o: ../lib/%.c $(OUT_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OUT_DIR):  
	@mkdir -p $(OUT_DIR)


asm: $(IMG)
	qemu-system-aarch64 -M raspi3 -kernel $(IMG) -display none -d in_asm
debug: $(IMG)
	qemu-system-aarch64 -M raspi3 -kernel $(IMG) -display none -S -s
run: $(IMG)
	qemu-system-aarch64 -M raspi3 -kernel $(IMG) -display none -serial null -serial stdio 
pty:
	qemu-system-aarch64 -M raspi3 -kernel $(IMG) -display none -serial null -serial pty -initrd ../initramfs.cpio -dtb ../raspi3/bcm2710-rpi-3-b-plus.dtb
clean:
	rm -rf $(OUT_DIR) $(ELF) $(IMG)