TARGET = bare_trap
CC = riscv64-unknown-linux-gnu-gcc
OBJCOPY = riscv64-unknown-linux-gnu-objcopy

CFLAGS = -mcmodel=medany -ffreestanding -nostdlib -O2
LDFLAGS = -T link.ld -nostdlib

# Only .c and .S/.s files as sources; link.ld is only for -T
SRCS = trap_handler.s main.c

$(TARGET).elf: $(SRCS) link.ld
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRCS)

run: $(TARGET).elf
	spike $<

clean:
	rm -f $(TARGET).elf