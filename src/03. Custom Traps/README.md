# Write Bare-Metal or PK-Based Programs to Handle Traps

### Note:

- 🔸 **Under PK (Proxy Kernel)**, **we cannot install our own trap handler**—PK owns M-mode and handles all traps.  
- 🔸 To **write our own trap handler**, we must run **bare-metal** (i.e., without PK), where our code runs directly in **M-mode**.

Since our goal includes **measuring context switches** and **studying low-level behavior**, writing a **minimal bare-metal program with a custom M-mode trap handler** is very valuable—even if simple.

We’ll do this in two parts:

---

## ✅ Part A: Understand PK’s Limitation (Trap Handling is Hidden)

When we run:
```bash
spike pk my_program
```
- `my_program` runs in **U-mode**.
- **PK runs in M-mode** and provides a fixed trap handler.
- We **cannot override** PK’s handler or access M-mode CSRs like `mtvec` from user code.

➡️ So for **custom trap handling**, we **skip PK** and run bare-metal.

---

## ✅ Part B: Write a Bare-Metal Program with a Custom Trap Handler

We’ll create a minimal RISC-V program that:
1. Runs in **M-mode** (no OS, no PK).
2. Sets up a **simple trap handler** in `mtvec`.
3. Triggers an `ecall` from M-mode.
4. Handles it and returns via `mret`.

This gives us full control over CSRs and trap flow.

---

### 📁 File Structure

We’ll create:
- `trap_handler.S` — assembly trap handler
- `main.c` — C code that triggers trap
- `link.ld` — linker script
- `Makefile` — to build

---

#### 1. 📄 `trap_handler.S`

```assembly
# trap_handler.S
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
    # Set stack pointer to 16KB above code start (safe in Spike)
    # Spike gives you ~128MB+ RAM starting at 0x80000000
    li sp, 0x80010000

    # Set mtvec to our handler
    la t0, trap_handler
    csrw mtvec, t0

    # Call a C function (optional)
    call main

    # Halt
1:  wfi
    j 1b

```
> ⚠️ Warning: Add newline at the end of the `trap_handler.s` file. Otherwise the make file reports a warning when compile time.

> ℹ️ Spike gives you ~128MB+ RAM starting at 0x80000000. So Set stack pointer to 16KB above code start (safe in Spike)
---

#### 2. 📄 `main.c`

```c
// main.c
void trigger_ecall() {
    asm volatile ("ecall");
}

void _exit(int code) {
    while(1); // simple halt
}

int main() {
    // Trigger an ecall from M-mode
    trigger_ecall();

    // If handler works, we return here
    _exit(0);
    return 0;
}
```

---

#### 3. 📄 `link.ld`

```ld
/* link.ld */
ENTRY(_start)
SECTIONS
{
    . = 0x80000000;
    .text : { *(.text) }
    .data : { *(.data) }
    .bss : { *(.bss) }
}
```

> 💡 Spike loads bare-metal programs at **0x80000000** by default.

---

#### 4. 📄 `Makefile`

```makefile
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
```

> ⚠️ Warning: Don’t list `link.ld` as a source file in the dependency list for the `.elf` target. Do not use `$^` in the `$(TARGET).elf: trap_handler.s main.c link.ld` line. `link.ld` is only for `-T`. 

> 🔸 We use `riscv64-unknown-linux-gnu-gcc` even for bare-metal—it works fine as long as we avoid OS assumptions (`-ffreestanding -nostdlib`).

---

#### 5. 🧪 Build and Run

```bash
make
make run
```

> ℹ️ This will create a `bare_trap.elf` file after the `make` command successfully completes.

> ℹ️ If any case we can clear the files after the `make` command by typing `make clear` command in the terminal.

✅ If our trap handler works, the program will:
- Set `mtvec` to `trap_handler`
- Execute `ecall`
- Trap to handler
- Increment `mepc` by 4
- Return via `mret`
- Reach `_exit` and hang (as expected)

---

### 🔍 Debug It in Spike

```bash
spike -d bare_trap.elf
```

In debugger:
```
(spike) until pc 0 0x80000020   # near ecall
(spike) r 1                     # execute ecall → trap! (Or we can just press the Enter key)
(spike) reg 0 mcause            # should be 11 (0xb) = M-mode ecall
(spike) reg 0 mepc              # should point to ecall address
```
