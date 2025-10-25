TARGET = bare_trap
CC = riscv64-unknown-linux-gnu-gcc
OBJCOPY = riscv64-unknown-linux-gnu-objcopy

CFLAGS = -mcmodel=medany -ffreestanding -nostdlib -O2
LDFLAGS = -T link.ld -nostdlib

$(TARGET).elf: trap_handler.s main.c link.ld
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

run: $(TARGET).elf
	spike $<

clean:
	rm -f $(TARGET).elf