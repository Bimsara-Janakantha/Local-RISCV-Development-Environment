TARGET = ctx_bare
CC = riscv64-unknown-linux-gnu-gcc

CFLAGS = -mcmodel=medany -ffreestanding -nostdlib -O2 -march=rv64imafd -mabi=lp64d
LDFLAGS = -T link.ld -nostdlib

# Only .c and .S/.s files as sources; link.ld is only for -T
SRCS = ctx_switch_bare.s main_bare.c

$(TARGET).elf: $(SRCS) link.ld
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRCS)

run: $(TARGET).elf
	spike $<

clean:
	rm -f $(TARGET).elf