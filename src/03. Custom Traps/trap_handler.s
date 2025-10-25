.section .text
.global trap_handler
.global _start

trap_handler:
    # Save registers (minimal: just ra and a0-a2 if needed)
    addi sp, sp, -32 # Stack free for 4 words
    sd ra, 0(sp)     # Save ra to the stack
    sd a0, 8(sp)     # Save a0 to the stack
    sd a1, 16(sp)    # Save a1 to the stack
    sd a2, 24(sp)    # Save a2 to the stack

    # Read mcause to decide action
    csrr a0, mcause
    csrr a1, mepc

    # Check if it's an ecall from M-mode (mcause = 0xb = 11)
    li a2, 11
    beq a0, a2, handle_ecall

    # Unknown trap: hang
    j trap_handler

handle_ecall:
    # Print a message? (We can't easily print bare-metal, so just increment a counter)
    # For now, just advance mepc past the ecall
    addi a1, a1, 4        # ecall is 4 bytes
    csrw mepc, a1

    # Restore
    ld ra, 0(sp)
    ld a0, 8(sp)
    ld a1, 16(sp)
    ld a2, 24(sp)
    addi sp, sp, 32

    mret

_start:
    # Set stack pointer (use top of memory)
    li sp, 0x80000000

    # Set mtvec to our handler
    la t0, trap_handler
    csrw mtvec, t0

    # Call a C function (optional)
    call main

    # Halt
1:  wfi
    j 1b