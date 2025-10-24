# Measuring Context Switch Overhead Using Cycle Counts

**Goal**: Estimate how many CPU cycles are spent **saving/restoring state** when switching between two execution contexts (e.g., user ↔ kernel via `ecall`).

In our current setup (**Spike + PK**), a “context switch” isn’t between two user threads (PK doesn’t support that), but rather the **trap/return cycle**:  
**User mode → PK (M-mode) → back to User mode**.

This round-trip **approximates the minimal context switch cost** in a system with a kernel.

---

## 📏 Step-by-Step: Measure Trap/Return Overhead

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

✅ Best choice: **`getpid`** (syscall number 170 in Linux/RISC-V). PK supports it and returns a dummy PID. 

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
    register long a7 asm("a7") = 170;  // SYS_getpid
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

### ✅ Your Task

1. **Implement `ctx_switch.c` / `enhanced_ctx_switch.c`** (with loop).
2. **Run it in Spike** and record the average cycle count.
3. **Try different syscalls** (`write`, `gettime`)—do they cost more?
4. **Answer**: Is the overhead dominated by register save/restore or syscall logic?

Once you have numbers, we can discuss:
- How this compares to real hardware
- How to reduce overhead (e.g., register windows, lazy FPU)
- Or move to **modifying register files** (your next topic)