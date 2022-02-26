CC = aarch64-linux-gnu-gcc
LD = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy

IMG = kernel8.img
ELF = kernel8.elf

SRC_DIR = src
OUT_DIR = out

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OUT_DIR)/%.o)

CFLAGS = -I include -c

.PHONY: asm debug clean run

$(IMG): $(ELF)
	$(OBJCOPY) -O binary $(ELF) $(IMG)
$(ELF): $(OBJS) linker.ld $(OUT_DIR)/entry.o
	$(LD) -T linker.ld -o $(ELF) $(OBJS) $(OUT_DIR)/entry.o

$(OUT_DIR)/entry.o: entry.S
	$(CC) $(CFLAGS) $< -o $@
$(OUT_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

asm: $(IMG)
	qemu-system-aarch64 -M raspi3 -kernel $(IMG) -display none -d in_asm
debug: $(IMG)
	qemu-system-aarch64 -M raspi3 -kernel $(IMG) -display none -S -s
run: $(IMG)
	qemu-system-aarch64 -M raspi3 -kernel $(IMG) -display none -serial null -serial stdio
clean:
	rm out/* $(ELF) $(IMG)