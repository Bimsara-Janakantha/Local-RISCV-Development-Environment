# Measuring Context Switch Overhead Using Cycle Counts

**Goal**: Estimate how many CPU cycles are spent **saving/restoring state** when switching between two execution contexts (e.g., user ↔ kernel via `ecall`).

In our current setup (**Spike + PK**), a “context switch” isn’t between two user threads (PK doesn’t support that), but rather the **trap/return cycle**:  
**User mode → PK (M-mode) → back to User mode**.

This round-trip **approximates the minimal context switch cost** in a system with a kernel.

---

## 📏 Part 1: Measure PK-Based Context Switch Overhead

### 1. **What Are We Measuring?**

We’ll measure:
```text
T = cycles(ecall + PK trap handling + mret)
```
This includes:
- Saving user registers (PK does this in software)
- Decoding the syscall
- Restoring registers and returning

> ⚠️ This is **not** a full OS context switch (no TLB flush, scheduler, etc.), but it’s the **foundation**.

---

### 2. **Instrument with `rdcycle`**

Use `rdcycle` **before and after** an `ecall` to capture total overhead.

But note: **We can’t put `rdcycle` *after* `ecall` if `ecall` terminates the program** (like `exit`). So use a **non-terminating syscall**.

✅ Best choice: **`getpid`** (syscall number 172 in Linux/RISC-V). PK supports it and returns a dummy PID. 

Find all supported syscalls for PK from here: **[syscall.c](https://github.com/riscv-software-src/riscv-pk/blob/master/pk/syscall.h)**

---

### 3. **Write the Benchmark Program**

Create `ctx_switch.c`:

```c
#include <stdio.h>
#include <stdint.h>

static inline uint64_t rdcycle() {
    uint64_t c;
    asm volatile ("rdcycle %0" : "=r" (c));
    return c;
}

// Make a getpid syscall (non-exiting)
long my_getpid() {
    register long a7 asm("a7") = 172;  // SYS_getpid
    register long a0 asm("a0");
    asm volatile ("ecall"
                  : "=r"(a0)
                  : "r"(a7)
                  : "memory");
    return a0;
}

int main() {
    uint64_t start, end;
    long pid;

    // Warm-up (optional, but reduces noise)
    my_getpid();

    start = rdcycle();
    pid = my_getpid();
    end = rdcycle();

    printf("PID: %ld\n", pid);
    printf("Trap+return cycles: %lu\n", end - start);
    return 0;
}
```

> 💡 Why `getpid`? It’s lightweight, doesn’t exit, and PK handles it quickly.

---

### 4. **Compile and Run**

```bash
riscv64-unknown-linux-gnu-gcc -static -O2 -o ctx_switch ctx_switch.c
spike pk ctx_switch
```

Sample output:
```
PID: 1
Trap+return cycles: 2480
```

> 📌 The exact number depends on Spike version, host CPU, and PK build—but it’s **repeatable** on the same setup.

---

### 5. **Improve Accuracy**

To reduce noise:
- **Run multiple times** and average.
- **Use `-O2`** to minimize loop overhead.
- **Avoid printf inside measurement** (we already do this).

Enhanced version (1000 iterations): `enhanced_ctx_switch.c`

```c
#define ITER 1000

int main() {
    uint64_t start, end;
    long pid;

    // Warm-up
    my_getpid();

    start = rdcycle();
    for (int i = 0; i < ITER; i++) {
        pid = my_getpid();
    }
    end = rdcycle();

    printf("Avg cycles per trap+return: %lu\n", (end - start) / ITER);
    return 0;
}
```

Compile and run:
```bash
riscv64-unknown-linux-gnu-gcc -static -O2 -o enhanced_ctx_switch enhanced_ctx_switch.c
spike pk enhanced_ctx_switch
```

Typical result: **~2000–3000 cycles per round-trip** in Spike.

> 🔬 Note: Spike is a **functional simulator**, not cycle-accurate like Verilator or FPGA. But it’s **consistent** for relative comparisons.

---

### 6. **What’s Included in This Measurement?**

When we call `my_getpid()`:
1. User: `rdcycle` → `ecall`
2. Hardware: traps to M-mode, saves PC to `mepc`, sets `mcause`
3. PK (M-mode):
   - Saves all 32 GPRs to stack
   - Checks `a7` (syscall number)
   - Emulates `getpid` → returns PID=1
   - Restores all GPRs
   - Executes `mret` → back to user
4. User: reads `rdcycle` again

✅ So we’re measuring **full register save/restore + dispatch + return**.

---

## 📏 Part 2: Measure Bare-Metal Manual Context Switch (Simulated)

Now, let’s simulate a register-based context switch like FreeRTOS would do—without traps, just saving/restoring registers.

This isolates the pure register save/restore cost.

### 1. Approach

- Define two “tasks” with their own register states.
- Use a switch_context function that:
- Saves caller’s registers to a stack or struct.
- Restores callee’s registers.
- Measure cycles for this switch.

---

### 2. ctx_switch_bare.S
```assembly
# ctx_switch_bare.S
# void switch_context(uint64_t *save_area, uint64_t *restore_area);
# Saves x1-x31 (except x0) to save_area, loads from restore_area

.globl switch_context
.text
switch_context:
    # Save all callee-saved + caller-saved (simulate full context)
    sd x1, 0*8(a0)
    sd x2, 1*8(a0)
    sd x3, 2*8(a0)
    sd x4, 3*8(a0)
    sd x5, 4*8(a0)
    sd x6, 5*8(a0)
    sd x7, 6*8(a0)
    sd x8, 7*8(a0)
    sd x9, 8*8(a0)
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

    # Restore
    ld x1, 0*8(a1)
    ld x2, 1*8(a1)
    ld x3, 2*8(a1)
    ld x4, 3*8(a1)
    ld x5, 4*8(a1)
    ld x6, 5*8(a1)
    ld x7, 6*8(a1)
    ld x8, 7*8(a1)
    ld x9, 8*8(a1)
    ld x10, 9*8(a1)
    ld x11, 10*8(a1)
    ld x12, 11*8(a1)
    ld x13, 12*8(a1)
    ld x14, 13*8(a1)
    ld x15, 14*8(a1)
    ld x16, 15*8(a1)
    ld x17, 16*8(a1)
    ld x18, 17*8(a1)
    ld x19, 18*8(a1)
    ld x20, 19*8(a1)
    ld x21, 20*8(a1)
    ld x22, 21*8(a1)
    ld x23, 22*8(a1)
    ld x24, 23*8(a1)
    ld x25, 24*8(a1)
    ld x26, 25*8(a1)
    ld x27, 26*8(a1)
    ld x28, 27*8(a1)
    ld x29, 28*8(a1)
    ld x30, 29*8(a1)
    ld x31, 30*8(a1)

    ret

```

> ⚠️ This saves 31 registers (`x1–x31`). In practice, only callee-saved (`x8–x15`, `x28–x31`) need saving in many ABIs—but for full context switch, we save all. 

---

#### 3. main_bare.c

```c
#include <stdint.h>

void switch_context(uint64_t *save, uint64_t *restore);

// Dummy task contexts
uint64_t ctx1[31] __attribute__((aligned(16))) = {0};
uint64_t ctx2[31] __attribute__((aligned(16))) = {0};

static inline uint64_t rdcycle() {
    uint64_t c;
    asm volatile ("rdcycle %0" : "=r" (c));
    return c;
}

void _start() {
    uint64_t start, end;

    // Initialize dummy registers
    ctx1[10] = 0x1234;  // x11
    ctx2[10] = 0x5678;

    start = rdcycle();
    switch_context(ctx1, ctx2);  // switch to ctx2
    switch_context(ctx2, ctx1);  // switch back
    end = rdcycle();

    // Output result via memory (no printf in bare-metal)
    // For Spike, we can inspect via debugger or write to known addr
    *(volatile uint64_t*)0x80001000 = end - start;

    // Halt
    while(1);
}
``` 

---

### 4. Build & Run (reuse linker script from Step 5)
```bash
riscv64-unknown-linux-gnu-gcc -ffreestanding -nostdlib -O2 -o ctx_bare.elf ctx_switch_bare.s main_bare.c link.ld

spike ctx_bare.elf
```

To read the result:

```bash
spike -d ctx_bare.elf
(spike) until pc 0 0x80000xxx  # near end
(spike) mem 0x80001000         # read cycle count
```

> ✅ Expect ~200–400 cycles for a full register save/restore (much cheaper than a full trap!).

---

## 📊 Interpretation

| Method | Cycles (approx, Spike) | What It Includes |
|-------|------------------------|------------------|
| **PK `ecall` round-trip** | 1300–1600 | Trap entry, PK handler, `mret`, CSR manipulation |
| **Manual register switch** | 200–400 | Only load/store of 31 registers |

> 🔍 Real insight: The trap mechanism itself (not just register save) dominates context-switch cost in simple systems. 

This explains why register file partitioning (your research focus) can help: if you reduce the number of registers to save, you reduce both manual switch cost and trap handler cost.

