CC=aarch64-linux-gnu-gcc
LD=aarch64-linux-gnu-ld
OBJCOPY=aarch64-linux-gnu-objcopy

IMG=kernel8.img
ELF=kernel8.elf

.PHONY: asm debug clean

$(IMG): $(ELF)
	$(OBJCOPY) -O binary $(ELF) $(IMG)
$(ELF): a.o linker.ld
	$(LD) -T linker.ld -o $(ELF) a.o
a.o: a.S
	$(CC) -c a.S

asm: $(IMG)
	qemu-system-aarch64 -M raspi3 -kernel $(IMG) -display none -d in_asm
debug: $(IMG)
	qemu-system-aarch64 -M raspi3 -kernel $(IMG) -display none -S -s
clean:
	rm a.o $(ELF) $(IMG)