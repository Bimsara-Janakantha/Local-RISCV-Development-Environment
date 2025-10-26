TARGET = ctx_bare
CC = riscv64-unknown-linux-gnu-gcc

CFLAGS = -mcmodel=medany -ffreestanding -nostdlib -O2 -march=rv64imafd -mabi=lp64d
LDFLAGS = -T link.ld -nostdlib

# Only .c and .s files as sources; link.ld is only for -T
SRCS = ctx_switch_bare.s main_bare.c

# Compile
$(TARGET).elf: $(SRCS) link.ld
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRCS)

# Run in debug mode
run: $(TARGET).elf
	spike -d $<

clean:
	rm -f $(TARGET).elf