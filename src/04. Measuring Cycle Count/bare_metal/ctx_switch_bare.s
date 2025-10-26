# ctx_switch_bare.S
# Bare-metal RISC-V (RV64) context switch benchmark
# Measures pure register save/restore cost

.section .text
.globl _start
.globl switch_context

# -------------------------------------------------
# void switch_context(uint64_t *save_area, uint64_t *restore_area);
# a0 = save_area, a1 = restore_area
# -------------------------------------------------
switch_context:
    # Save x1–x31 (x0 is hardwired zero, skip)
    sd x1,  0*8(a0)
    sd x2,  1*8(a0)
    sd x3,  2*8(a0)
    sd x4,  3*8(a0)
    sd x5,  4*8(a0)
    sd x6,  5*8(a0)
    sd x7,  6*8(a0)
    sd x8,  7*8(a0)
    sd x9,  8*8(a0)
    sd x10, 9*8(a0)
    sd x11, 10*8(a0)
    sd x12, 11*8(a0)
    sd x13, 12*8(a0)
    sd x14, 13*8(a0)
    sd x15, 14*8(a0)
    sd x16, 15*8(a0)
    sd x17, 16*8(a0)
    sd x18, 17*8(a0)
    sd x19, 18*8(a0)
    sd x20, 19*8(a0)
    sd x21, 20*8(a0)
    sd x22, 21*8(a0)
    sd x23, 22*8(a0)
    sd x24, 23*8(a0)
    sd x25, 24*8(a0)
    sd x26, 25*8(a0)
    sd x27, 26*8(a0)
    sd x28, 27*8(a0)
    sd x29, 28*8(a0)
    sd x30, 29*8(a0)
    sd x31, 30*8(a0)

    # Restore x1–x31 from restore_area
    ld x1,  0*8(a0)
    ld x2,  1*8(a0)
    ld x3,  2*8(a0)
    ld x4,  3*8(a0)
    ld x5,  4*8(a0)
    ld x6,  5*8(a0)
    ld x7,  6*8(a0)
    ld x8,  7*8(a0)
    ld x9,  8*8(a0)
    ld x10, 9*8(a0)
    ld x11, 10*8(a0)
    ld x12, 11*8(a0)
    ld x13, 12*8(a0)
    ld x14, 13*8(a0)
    ld x15, 14*8(a0)
    ld x16, 15*8(a0)
    ld x17, 16*8(a0)
    ld x18, 17*8(a0)
    ld x19, 18*8(a0)
    ld x20, 19*8(a0)
    ld x21, 20*8(a0)
    ld x22, 21*8(a0)
    ld x23, 22*8(a0)
    ld x24, 23*8(a0)
    ld x25, 24*8(a0)
    ld x26, 25*8(a0)
    ld x27, 26*8(a0)
    ld x28, 27*8(a0)
    ld x29, 28*8(a0)
    ld x30, 29*8(a0)
    ld x31, 30*8(a0)

    ret

# -------------------------------------------------
# _start: entry point for bare-metal
# -------------------------------------------------
_start:
    # Set stack pointer to top of RAM (Spike: 128 MiB @ 0x80000000 → top = 0x88000000)
    li sp, 0x88000000

    # Call C main
    call main

    # Halt forever
1:  wfi
    j 1b
