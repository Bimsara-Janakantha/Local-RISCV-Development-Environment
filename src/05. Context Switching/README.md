# Simulate context switching

Let’s clarify your goal and options.

---

## 🔍 What Is "Actual" Context Switching?

In a real OS (like FreeRTOS or Linux), a context switch involves:
1. **Taking a trap** (e.g., timer interrupt or `ecall`).
2. **Saving the full CPU state** of the current task:
   - All registers (`x0–x31`, though `x0` is hardwired to 0)
   - Program counter (`sepc` or `mepc`)
   - Status (`sstatus`/`mstatus`)
3. **Switching to a new task’s stack**.
4. **Restoring the new task’s full CPU state**.
5. **Returning via `sret`/`mret`**, which resumes execution at the saved PC.

This **cannot be done in pure user mode** — you need **Supervisor (S-mode) or Machine (M-mode)** code.

---

## ✅ Your Options to Simulate *Actual* Context Switching

Since you’re using **Spike**, you have **three main paths**:

---

### 🚩 Option 1: **Bare-Metal M-Mode Kernel (Recommended for Learning)**

**What**: Write a tiny OS that runs directly on Spike **without PK**.  
**Privilege**: Runs in **M-mode** (Machine mode).  
**Pros**:
- Full control over traps, CSRs, and context switching.
- You implement **real trap handlers** and **`mret`-based resumption**.
- Perfect for measuring **true context-switch cost**.
- Aligns with your research on register partitioning.

**Cons**:
- No `printf` unless you implement UART or use Spike’s HTIF (host I/O).
- No C runtime (you must set up stack, `.data`, `.bss` yourself).

**Tools**:
- `riscv64-unknown-elf-gcc` (bare-metal toolchain)
- Custom linker script
- Handwritten trap handler in `.S`

> ✅ **Best for your goal**: You can measure **exact cycles** for full context switch, including trap entry/exit.

---

### 🚩 Option 2: **S-Mode Kernel with Proxy Kernel Replacement**

**What**: Replace PK with your own **S-mode kernel** (like a minimal FreeRTOS).  
**Privilege**: Tasks run in **U-mode**, kernel in **S-mode**.  
**Pros**:
- Closer to real-world RTOS (e.g., FreeRTOS on RISC-V).
- You handle **`ecall`**, **timer interrupts**, and **`sret`**.
- Can still use Spike’s HTIF for I/O.

**Cons**:
- More complex (need PMP, delegation, etc.).
- Requires setting up `stvec`, `sie`, `sip`, etc.

> 🔸 Good if you plan to port to real hardware later.

---

### 🚩 Option 3: **Use Existing RTOS (e.g., FreeRTOS) on Spike**

**What**: Run FreeRTOS (ported to RISC-V) on Spike.  
**Pros**:
- Real, optimized context switch.
- You can **instrument** the context switch to measure cycles.
- Leverages existing work.

**Cons**:
- Less control over low-level details.
- Harder to modify register-save logic for partitioning experiments.

> 🔸 Good for **validation**, but less ideal for **research experimentation**.

---

## 🎯 Recommendation: **Start with Option 1 (Bare-Metal M-Mode)**

Given your research focus (**register partitioning**, **cycle-accurate measurement**), **Option 1** gives you:
- Full control over what registers are saved.
- Ability to measure **trap overhead + register save/restore**.
- No interference from PK or OS abstractions.

---

## 🛠️ Step-by-Step Plan: Bare-Metal M-Mode Context Switch

### 🔹 Step 1: Switch Toolchain
Use **`riscv64-unknown-elf-gcc`** (not `linux-gnu`):
```bash
# Install if needed
git clone https://github.com/riscv/riscv-gnu-toolchain
./configure --prefix=/opt/riscv --enable-multilib
make -j$(nproc)
```

> Why? `linux-gnu` assumes a POSIX environment (glibc, dynamic linking). `elf` is for bare-metal.

---

### 🔹 Step 2: Minimal Runtime Setup

Create:
- `crt0.S`: Startup code (set stack, clear BSS, call `main`)
- `linker.ld`: Linker script (define memory layout, stack)
- `main.c`: Your C code

Example `crt0.S`:
```assembly
.section .text.boot
.globl _start
_start:
    la sp, _stack_top
    call main
1:  j 1b  # hang if main returns
```

Example `linker.ld`:
```ld
ENTRY(_start)
MEMORY {
    ram : ORIGIN = 0x80000000, LENGTH = 128M
}
SECTIONS {
    . = 0x80000000;
    .text : { *(.text.boot) *(.text) }
    .data : { *(.data) }
    .bss : { *(.bss COMMON) }
    _stack_top = ORIGIN(ram) + LENGTH(ram);
}
```

---

### 🔹 Step 3: Implement Trap Handler

In `trap.S`:
```assembly
.globl trap_handler
trap_handler:
    # Save all registers (or subset for partitioning)
    addi sp, sp, -16*8
    sd x1, 0*8(sp)
    sd x2, 1*8(sp)
    # ... save x3–x31 (skip x0)
    sd x31, 30*8(sp)

    # Save mepc, mstatus
    csrr t0, mepc
    sd t0, 31*8(sp)
    csrr t1, mstatus
    sd t1, 32*8(sp)

    # Call C handler
    mv a0, sp
    call handle_trap

    # Restore
    ld t0, 32*8(sp)
    csrw mstatus, t0
    ld t0, 31*8(sp)
    csrw mepc, t0
    # ... restore x1–x31
    addi sp, sp, 16*8
    mret
```

Set `mtvec` in `main()`:
```c
void main() {
    uintptr_t trap_entry = (uintptr_t)trap_handler;
    asm volatile ("csrw mtvec, %0" :: "r"(trap_entry));
    // ...
}
```

---

### 🔹 Step 4: Implement Context Switch in `handle_trap`

In C:
```c
void handle_trap(uintptr_t *saved_regs) {
    static int current = 0;
    // saved_regs[31] = mepc, [32] = mstatus

    if (should_switch_task()) {
        // Save current task's context (pointed by saved_regs)
        task_ctx[current] = saved_regs;

        // Switch to next task
        current = 1 - current;

        // Copy next task's context into saved_regs
        // (so mret resumes it)
        for (int i = 0; i < 33; i++) {
            saved_regs[i] = task_ctx[current][i];
        }
    }
}
```

> 🔥 This is the **core of real context switching**: you **overwrite the trap frame** so `mret` resumes a different task.

---

### 🔹 Step 5: Trigger Switch via Timer or `ecall`

- Use **machine timer interrupt** (`mtip`) for preemptive switching.
- Or use `ecall` for cooperative switching.

Example (cooperative):
```c
#define ECALL_SWITCH 99
void yield(void) {
    register long a7 = ECALL_SWITCH;
    asm volatile ("ecall" :: "r"(a7));
}
```

In `handle_trap`, check `mcause`:
```c
if (mcause == 0x0000000000000008) { // environment call
    if (a7 == ECALL_SWITCH) { /* do switch */ }
}
```

---

### 🔹 Step 6: Measure Cycles

Use `rdcycle` **before `ecall`** and **after `mret`** (tricky), or:
- Read `cycle` CSR in trap handler before/after switch.
- Or use Spike’s `--log-commits` to count instructions.

---

## 🧪 Build & Run

```bash
riscv64-unknown-elf-gcc -T linker.ld -nostdlib -o kernel crt0.S trap.S main.c
spike kernel
```

> No `pk` needed!

---

## ✅ Why This Is "Actual" Context Switching

- ✅ Runs in **M-mode** (true kernel).
- ✅ Uses **trap/`mret`** for control transfer.
- ✅ Saves **full PC (`mepc`)** and **all registers**.
- ✅ Switches **stacks** (each task has its own).
- ✅ Measures **real overhead** (trap entry + save/restore + `mret`).

---

## 📚 Next Steps

1. Start with a **minimal M-mode "hello world"** on Spike (no PK).
2. Add a **trap handler that prints `mcause`**.
3. Implement **cooperative switch via `ecall`**.
4. Add **cycle counting**.
5. Experiment with **saving fewer registers** (partitioning).

---

Would you like:
- A complete minimal bare-metal context switch example?
- Help setting up the toolchain and linker script?
- Guidance on measuring cycles accurately in M-mode?

Let me know — I’ll provide the exact files and commands!
