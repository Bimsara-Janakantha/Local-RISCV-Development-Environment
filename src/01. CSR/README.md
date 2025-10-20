# Control and Status Registers (CSRs)

### 📌 What Are CSRs?
CSRs (**Control and Status Registers**) are special-purpose registers in RISC-V that control or report the processor state. They are **not part of the general-purpose register file (x0–x31)** and are accessed via special instructions:
- `csrrw rd, csr, rs1` — CSR read/write -> Read CSR into rd
- `csrrs rd, csr, rs1` — CSR read and set bits
- `csrrc rd, csr, rs1` — CSR read and clear bits
- Pseudo-instructions like `csrr rd, csr` and `csrw csr, rs1` are commonly used.

CSRs are **privilege-level sensitive**:  
- **User mode (U)**: Very limited CSR access (e.g., `cycle`, `time` if delegated).
- **Supervisor mode (S)**: More CSRs (e.g., `sstatus`, `sie`, `stvec`).
- **Machine mode (M)**: Full access (e.g., `mstatus`, `mtvec`, `mepc`, `mcause`).

> ⚠️ **Under PK (Proxy Kernel)**, your C program runs in **user mode**, so you can only access **user-readable CSRs**—unless PK delegates access (e.g., to `cycle`).

---

### ✅ Key CSRs You Should Know

| CSR | Address | Mode | Purpose |
|-----|---------|------|--------|
| `cycle` |  0xC00 | U/S/M | Counts clock cycles (lower 32 bits in `cycleh` for 64-bit) |
| `time` |  0xC01 | U/S/M | Real-time clock (if available) |
| `mstatus` |  0x300 | M | Global interrupt enable, privilege mode status |
| `mepc` |  0x341 | M | PC where trap occurred |
| `mcause` |  0x342 | M | Reason for trap (interrupt vs exception) |
| `mtvec` |  0x305 | M | Trap vector base address |
| `mie` |  0x304 | M | Interrupt enable bits |
| `mip` |  0x344 | M | Interrupt pending bits |
| `sstatus` |  0x100 | S | Supervisor status (subset of mstatus) |
| `stvec` |  0x105 | S | Supervisor trap vector |

> 🔍 In **user mode under PK**, you can safely read `cycle` and `time` because **PK delegates access** to them by setting bits in `mcounteren`.

---

### 🧪 Practical: Read `cycle` CSR in C (User Mode)

Create a file `csr_cycle.c`:

```c
#include <stdio.h>
#include <stdint.h>

static inline uint64_t rdcycle() {
    uint64_t result;
    asm volatile ("rdcycle %0" : "=r" (result));
    return result;
}

int main() {
    uint64_t c1 = rdcycle();
    // Do some work
    for (volatile int i = 0; i < 1000; i++);
    uint64_t c2 = rdcycle();
    printf("Start cycle: %lu\n", c1);
    printf("End cycle:   %lu\n", c2);
    printf("Elapsed:     %lu cycles\n", c2 - c1);
    return 0;
}
```

Compile and run:
```bash
riscv64-unknown-linux-gnu-gcc -o csr_cycle csr_cycle.c
spike pk csr_cycle
```

✅ You should see increasing cycle counts.

> 💡 `rdcycle` is a **pseudo-instruction** that maps to `csrr t0, cycle`.

---

### 🚫 What You *Can’t* Do in User Mode (Under PK)

Try reading `mstatus` in user mode:

```c
uint64_t read_mstatus() {
    uint64_t val;
    asm volatile ("csrr %0, mstatus" : "=r" (val));
    return val;
}
```

➡️ This will cause an **illegal instruction exception**, because user mode cannot access `mstatus`. Spike will terminate with a trap.

---

### 🔍 How to Inspect CSR State in Spike

Use Spike’s debug mode to see CSR values during execution:

```bash
spike -d pk csr_cycle
```

Then in the debugger:
```
: until pc 0 0x10xxx  # run until your code
: reg 0 cycle         # read cycle CSR
: reg 0 mstatus       # (only works if in M-mode)
: quit
```

Or log all instructions and CSR changes:
```bash
spike --log-commits pk csr_cycle
```

---

### 📚 Next Step Preview: Traps & Interrupts (Step 4)

Once you’re comfortable reading/writing **allowed CSRs**, you’ll:
- Trigger a trap (e.g., `ecall`)
- See how PK handles it
- Learn how `mepc`, `mcause`, and `mtvec` work (in M-mode)
- Eventually write your own minimal trap handler (beyond PK)

---

### ✅ Your Task Now

1. **Run the `csr_cycle.c` example** and verify it works.
2. **Try reading `time`** using `rdtime` (similar to `rdcycle`).
3. **Attempt to read `mstatus`** and observe the illegal instruction trap.
4. **Check the RISC-V Privileged Spec** (Section 2.8 and 3.1) for CSR descriptions.

When you’re done or have questions (e.g., “Why can I read `cycle` but not `mstatus`?”), let me know—we’ll move to **Step 4: Traps and Interrupts**.
