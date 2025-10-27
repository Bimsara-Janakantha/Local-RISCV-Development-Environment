# Traps and Interrupts

### 🎯 What Are Traps

In RISC-V, a **[trap](https://osblog.stephenmarz.com/ch4.html)** is a general term for any **synchronous exception** (e.g., illegal instruction, `ecall`) or **asynchronous interrupt** (e.g., timer, external). When a trap occurs, the processor:
1. **Saves** the current program counter (PC) to a CSR (`mepc` or `sepc`).
2. **Records** the trap cause in `mcause`/`scause`.
3. **Disables** further interrupts (via `mstatus.MIE` or `sstatus.SIE`).
4. **Jumps** to a **trap handler** whose address is in `mtvec`/`stvec`.

> **Key idea**: Traps transfer control from a lower privilege mode (e.g., U) to a higher one (e.g., M or S).

Since we’re using **Spike + PK**, our user program runs in **U-mode**, and **PK runs in M-mode** and handles all traps on our behalf.

---

### 🧩 Types of Traps

| Type | Example | Synchronous? | Handled by |
|------|--------|--------------|-----------|
| **Environment Call** | `ecall` from U/S/M | ✅ Yes | OS / PK |
| **Illegal Instruction** | Invalid opcode | ✅ Yes | M-mode |
| **Load/Store Fault** | Bad address | ✅ Yes | M-mode |
| **Machine Timer Interrupt** | `mtip` set | ❌ No | M-mode |
| **External Interrupt** | I/O event | ❌ No | M-mode |

> In our current setup (**Spike + PK**), **all traps go to PK’s M-mode handler**.

---

### 🔑 Trap Control Registers

[Trap Codes](https://dram.page/riscv-trap/)
- `mtvec`: machine trap vector (base address of trap handler)
- `mepc`: machine exception program counter (return address)
- `mcause`: encodes the reason (interrupt or exception)
- `mtval`: extra info (e.g., bad address)
- `mstatus`: interrupt-enable bits and privilege state

---

### 🔍 Trap Handling Flow

When your C program calls `printf()`, `exit()`, or even `_exit()`, it eventually triggers an **`ecall` instruction** from **user mode**.

1. CPU traps to **M-mode** (since PK runs in M-mode).
2. PK sees `mcause = 8 + 0` (8 = user environment call).
3. PK emulates the system call (e.g., writes to host stdout).
4. PK executes `mret` to return to user mode at `mepc`.


---

### 🧪 Practical: Trigger an `ecall` Manually

Create `ecall_exit.c`:

```c
#include <stdio.h>

// Make a raw ecall (will be handled by PK as exit syscall)
void my_exit(int code) {
    register long a7 asm("a7") = 93;   // Linux/RISC-V exit syscall number
    register long a0 asm("a0") = code;
    asm volatile ("ecall" : : "r"(a0), "r"(a7) : "memory");
}

int main() {
    printf("About to trigger ecall...\n");
    my_exit(0);  // This calls ecall
    // Never reaches here
    return 0;
}
```

Compile and run:
```bash
riscv64-unknown-linux-gnu-gcc -o ecall_exit ecall_exit.c
spike pk ecall_exit
```

✅ Output:
```
About to trigger ecall...
```
…and exits cleanly.

> 💡 PK interprets `a7=93` as the `exit` system call (matches Linux ABI).

---

### 📊 Observe Trap Behavior in Spike

Run in debug mode to see the trap:

```bash
spike -d pk ecall_exit
```

In the debugger:
```
: until pc 0 0x00010xxx   # get close to ecall
: r                       # view registers
: until insn 0 ecall      # run until ecall instruction
```

After `ecall`:
- Check `mepc`: should point to the `ecall` instruction.
- Check `mcause`: should be `0x0000000000000008` (user ecall).
- Execution jumps to PK’s trap handler (address in `mtvec`).

You can inspect:
```
: reg 0 mepc
: reg 0 mcause
: reg 0 mtvec
```

---

### ⚙️ What Happens During a Trap? (M-Mode View)

When a trap occurs in M-mode (as with PK), the hardware **automatically**:
- Sets `mepc` = PC of faulting instruction (or next instruction for interrupts).
- Sets `mcause`:
  - Bit 63 = 1 → interrupt
  - Bit 63 = 0 → exception
  - Lower bits = cause code (e.g., 8 = user ecall)
- Clears `mstatus.MIE` (disables interrupts).
- Jumps to `mtvec`:
  - If `mtvec[1:0] == 00`: direct mode → jump to `mtvec`
  - If `mtvec[1:0] == 01`: vectored mode → jump to `mtvec + 4 * cause`

PK uses **direct mode**, so all traps go to one handler.

---

### 🛑 Interrupts in Spike + PK

By default, **Spike enables a timer interrupt** periodically. PK handles it to support time-based syscalls.

You can **disable interrupts** in your user code (but it won’t matter—PK controls M-mode).

To see timer interrupts:
```bash
spike --isa=rv64gc --pc=0x10000 --log-commits pk ecall_exit | grep trap
```
You’ll see periodic `trap` entries with `mcause = 0x8000000000000007` (machine timer interrupt).

---

### 📚 Key CSRs for Traps (Recap)

| CSR | Role |
|-----|------|
| `mepc` | Return address after trap |
| `mcause` | Trap reason (interrupt/exception + code) |
| `mtvec` | Trap handler entry point |
| `mstatus` | Tracks previous mode (MPP bits) and interrupt enable (MIE) |

---

### ✅ Your Task Now

1. **Run `ecall_exit.c`** and confirm it works.
2. **Use `spike -d`** to observe `mepc` and `mcause` right after the `ecall`.
3. **Modify the program** to trigger an **illegal instruction** (e.g., `.word 0x0` in inline asm) and see how PK handles it (should exit with error).
4. **Read RISC-V Privileged Spec, Section 3.1.6 (Trap Handling)**.
